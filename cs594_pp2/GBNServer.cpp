#include "GBNServer.hpp"
#include <pthread.h>

GBNServer::GBNServer(PacketStreamer &streamer, PAYLOAD_LENGTH_T lmax_packet_size, int ack_timeout, int size) : handler(this), shared_ctx(streamer)
{
  shared_ctx.last_window = 0;
  shared_ctx.q_size = size;
  shared_ctx.timeout = ack_timeout;
  max_packet_size = lmax_packet_size;
  shared_ctx.acktimer.CreateTimer(&handler);
  shared_ctx.cond = PTHREAD_COND_INITIALIZER;
  shared_ctx.win_mutex = PTHREAD_MUTEX_INITIALIZER;
}

int GBNServer::Push(SPacket sp)
{
  queue<SPacket> window;
  SEQUENCE_NUMBER_T seqn;
  pthread_mutex_lock(&shared_ctx.win_mutex);
  if(shared_ctx.q.size()>0)
  {
    printf("\nPUSH STATE FRONT: %c\n",(char)shared_ctx.q.front().state);
    if(shared_ctx.q.front().state == State::EXPIRED)
    {
      Socket::copy_ntoh(seqn,*shared_ctx.q.front().pkt.GetOffsetPtr<const SEQUENCE_NUMBER_T>(enum_cast(DataACKPacketFields::SEQ_NUM)));
      shared_ctx.q.front().state = State::SENT;
      while(&shared_ctx.q.back() != &shared_ctx.q.front())
      {
	if(shared_ctx.acktimer.IsSet == 0)
	{
	  printf("%04u\n", seqn);
	  shared_ctx.sn = seqn;
	  shared_ctx.acktimer.SetTimer(shared_ctx.timeout);
	  shared_ctx.acktimer.IsSet = 1;
	}
	shared_ctx.server_streamer.SendPacket(shared_ctx.q.front().pkt);
        window.push(shared_ctx.q.front());
	shared_ctx.q.pop();
	printf(":%c:", (char)window.back().state);
      }
      shared_ctx.q = window;
      printf("window size: %d %d\n", (int)window.size(), (int)shared_ctx.q.size());
//      SPacket tmp = shared_ctx.q.pop();
    }
  }

  printf("push: q.size() %d\n", (int)shared_ctx.q.size());
  if(((int)shared_ctx.q.size()) == shared_ctx.q_size)
  {
    printf("wait: window is full\n");
    pthread_cond_broadcast(&shared_ctx.cond);
    pthread_cond_wait(&shared_ctx.cond,&shared_ctx.win_mutex);
  }
  if(((int)shared_ctx.q.size()) < shared_ctx.q_size)
  {
    shared_ctx.q.push(sp);
    pthread_cond_broadcast(&shared_ctx.cond);
  }
  pthread_mutex_unlock(&shared_ctx.win_mutex);
  return 0;
}

void SharedContext::SetStateLPkt(State s)
{
  q.back().state = s;
  printf("state: %c\n", (char)q.back().state);
}

int GBNServer::SendStream(istream *input_stream, SEQUENCE_NUMBER_T &init_seq)
{
  int retval = 0;
  Packet ack_pkt;
  PAYLOAD_LENGTH_T bytes_read;
  seqnum = init_seq;
  printf("Starting File transfer using GO-Back-N ARQ..\n");
  receiver.start(&shared_ctx);
  while (input_stream->good())
  {
    bytes_read = max_packet_size;
    shared_ctx.pktmutex.lock();
    PacketStreamer::InitDataPacket(shared_ctx.send_pkt, seqnum, input_stream, bytes_read);
    SPacket sp(shared_ctx.send_pkt, State::AVAILABLE);
    Push(sp);
    if (shared_ctx.acktimer.IsSet == 0 && retval == 0)
    { printf("timer is set!!!\n");
      shared_ctx.acktimer.SetTimer(shared_ctx.timeout);
      shared_ctx.acktimer.IsSet = 1;
      retval = 1;
    }
    shared_ctx.server_streamer.SendPacket(shared_ctx.send_pkt);
    shared_ctx.SetStateLPkt(State::SENT);

    printf("D[%04u](%u)->", seqnum, bytes_read);
    shared_ctx.pktmutex.unlock();

    fflush(stdout);
    seqnum++;
  }
  shared_ctx.last_window=1;
  init_seq = seqnum;
  return retval;
}

void ack_handler::operator()()
{ 
  printf("##################################>>>>>TIMER ACKED\n");
//  server_ptr->shared_ctx.pktmutex.lock();
  pthread_mutex_lock(&server_ptr->shared_ctx.win_mutex);
  server_ptr->shared_ctx.acktimer.IsSet = 0;
  if(server_ptr->shared_ctx.q.size()==0)
  {
    pthread_cond_broadcast(&server_ptr->shared_ctx.cond);
    pthread_cond_wait(&server_ptr->shared_ctx.cond, &server_ptr->shared_ctx.win_mutex);
  }
  while(server_ptr->shared_ctx.q.front().state == State::ACKNOWLEDGED)
  {
    server_ptr->shared_ctx.q.pop();
    printf("poped\n");
  }
  if(server_ptr->shared_ctx.q.front().state == State::SENT)
  {
    server_ptr->shared_ctx.q.front().state = State::EXPIRED;
    printf("set to expired\n");
  }
  pthread_cond_broadcast(&server_ptr->shared_ctx.cond);
  pthread_mutex_unlock(&server_ptr->shared_ctx.win_mutex);
//  server_ptr->shared_ctx.pktmutex.unlock();
}

void GBNServer::SetWindowSize(int size)
{
  shared_ctx.q_size = size;
}


void* handler_receiver(void* v)
{
  printf("==================\n");
  SharedContext *sctx = reinterpret_cast<SharedContext*>(v);
  Packet ack_pkt;
  SEQUENCE_NUMBER_T pkt_seqnum;
  SEQUENCE_NUMBER_T seqnum = sctx->seqnum;
  int i =0;
//  sctx->pktmutex.lock();
  while(sctx->last_window != 1 || i <= 16)
  {
    i++;
    sctx->server_streamer.RecvPacket(ack_pkt);
    sctx->acktimer.CancelTimer();

    pthread_mutex_lock(&sctx->win_mutex);
    sctx->acktimer.IsSet = 0;
    if (ack_pkt.isType(PacketType::DATAACK_PACKET_TYPE))
    {
      Socket::copy_ntoh(pkt_seqnum, *ack_pkt.GetOffsetPtr<const SEQUENCE_NUMBER_T>(enum_cast(DataACKPacketFields::SEQ_NUM)));
      printf("pkt_seqnum: %d seqnum: %d\n", (int)pkt_seqnum, (int)seqnum);
      
      if (pkt_seqnum == seqnum)
      {
        //ACCEPTED ACK
        printf("A[%04u]\n", seqnum);
	sctx->SetStateLPkt(State::ACKNOWLEDGED);
        seqnum++;
	pthread_cond_broadcast(&sctx->cond);
      }
      else
      {
        //DROPPED ACK
        printf("A[%04u] Dup:%04u\n*", pkt_seqnum, seqnum);
      }
    } else {
      printf("this is not data ack packet type\n");
    }
//    pthread_mutex_lock(&sctx->win_mutex);
    if(((int)sctx->q.size()) != sctx->q_size)
    {
      pthread_cond_broadcast(&sctx->cond);
      pthread_cond_wait(&sctx->cond, &sctx->win_mutex);
    }
    if(sctx->q.size() == 0){
      pthread_cond_broadcast(&sctx->cond);
      pthread_cond_wait(&sctx->cond, &sctx->win_mutex);
    }
    pthread_mutex_unlock(&sctx->win_mutex);
  }
//  sctx->pktmutex.unlock();
  return 0;
}

void Receiver::start(SharedContext *sc)
{
  s_context = sc;
  int r = pthread_create(&threadid, NULL, handler_receiver, s_context);
  if(r!=0)
    printf("error: r != 0");
  printf("start routine\n");
}

SPacket::SPacket(Packet &p, State s) : state(s), pkt(p)
{};
