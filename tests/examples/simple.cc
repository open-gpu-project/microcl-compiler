[[microcl::gpu]]
int gpu_function() {
   return 0;
}

[[microcl::driver]]
int driver_function() {
   (void) gpu_function();
   return 0;
}

int main() {
   (void) driver_function();
   return 0;
}
