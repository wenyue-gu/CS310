
#define VIDEO_MEMORY    (void*) 0xB8000

void
umain()
{
	char* badhello = "badhello ";
	for(int i = 0; i < 80*24 - 9; i+=9) {
		for(int j = 0 ; j < 9; j++) {
			char* output_pos = (char*) VIDEO_MEMORY + 2*i + 2*j;
			*output_pos = badhello[j];
		}
	}

	while(1) { }
}
