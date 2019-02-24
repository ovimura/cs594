#include<iostream>

enum class MY_TYPE : size_t
{
  PACKET_TYPE = 3
};


int main()
{
  MY_TYPE m = MY_TYPE::PACKET_TYPE;
  std::cout<< "THis is my type enum class: "<< static_cast<std::underlying_type<MY_TYPE>::type>(m) << std::endl;
  return 1;
}
