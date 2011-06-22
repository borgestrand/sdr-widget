void set_local_audio_device(char * device);
int open_local_audio();
int close_local_audio();
int write_local_audio(float* left_samples,float* right_samples,int samples,int increment);

