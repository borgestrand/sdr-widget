void set_port_audio_device(int device);
void open_port_audio();
void close_port_audio();
void write_port_audio(float* left_samples,float* right_samples,int samples,int increment);
