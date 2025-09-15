[[microcl::gpu]]
int gpu_function(int x) {
   return x + 10;
}

[[microcl::driver]]
void driver_function(int* buf_in, int n, int* buf_out) {
   for (int i = 0; i < n; ++i) {
      buf_out[i] = gpu_function(buf_in[i]);
   }
}

int main() {
   int n = 5;
   int buf_in[5] = {1, 2, 3, 4, 5};
   int buf_out[5] = {0};
   driver_function(buf_in, n, buf_out);
   return 0;
}
