void _start(void) {
  volatile int counter = 0;
  while (1) {
    counter++;
    for (volatile int i = 0; i < 100000; i++);
  }
}
