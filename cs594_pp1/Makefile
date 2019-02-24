all: cs494rcp_client cs494rcp_server

cs494rcp_client:
	gcc -o cs494rcp_client cs494rcp_client.c utils.c -lrt -pthread

cs494rcp_server:
	gcc -o cs494rcp_server cs494rcp_server.c utils.c -lrt -pthread

clean:
	$(RM) cs494rcp_client cs494rcp_server
