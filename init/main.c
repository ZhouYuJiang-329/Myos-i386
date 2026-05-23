//
// Created by ziya on 22-6-23.
//

void kernel_main(void) {
    char* video = (char*)0xb8000;
      char* msg = "Hello OS!";
      int i = 0;

      while (msg[i] != '\0') {
          video[i * 2] = msg[i];      // 字符
          video[i * 2 + 1] = 0x0A;    // 属性（白字黑底）
          i++;
      }

}