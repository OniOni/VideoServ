int get_fragment(char image[], int len, int start, int end, char ** frag)
{
  int size = end - start;
  *frag = malloc(size * sizeof(char));
  int read = 0;

  for (int i = 0; i < size; i++)
  {
    if (start + i < len)
    {
      frag[i] = image[start + i];
      read++;
    }
  }

  return read;
}

int send_image_udp(int sock, int image, int frag_size)
{
  int len_ima, len, sent, read, start;
  char str[12], *buff_ima;
  sprintf(str, "%d.jpg", image);

  file_to_buffer(str, &buff_ima, &len_ima);

  char * buff;

  do
  {
    //get next fragment
    read = get_fragment(buff_ima, len_ima, start, start+frag_size, &buff);
    
    //build "header"
    sprintf(buff, "%d\r\n%d\r\n%d\r\s", image, len_ima, pos_pack, read);
    perror("sprintf");
      
    sent = len = strlen(buff);

    //send "header"
    puts("Going to send image\n");
    do{
      sent -= send(sock, buff, len, 0);
    }
    while(sent > 0);
    
    sent = len = len_ima;
    //send fragment
    do{
      sent -= send(sock, buff, read, 0);
    }
    while(sent > 0);

    start += read;
  }
  while(read == frag_size);
}
