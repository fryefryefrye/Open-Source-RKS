#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	FILE* fphzk = NULL;
	int i, j, k, offset;
	int flag;
	unsigned char buffer[32];
	unsigned char word[5];
	unsigned char key[8] = {
		0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
	};
	fphzk = fopen("hzk16", "rb");
	if(fphzk == NULL){
		fprintf(stderr, "error hzk16\n");
		return 1;
	}
	while(1){
		printf("输入要生成字模的汉字(多个):");
		for(;;){
			fgets((char*)word, 3, stdin);
			if(*word == '\n') 
				break;
			offset = (94*(unsigned int)(word[0]-0xa0-1)+(word[1]-0xa0-1))*32;
			fseek(fphzk, offset, SEEK_SET);
			fread(buffer, 1, 32, fphzk);
			for(k=0; k<16; k++){
				for(j=0; j<2; j++){
					for(i=0; i<8; i++){
						flag = buffer[k*2+j]&key[i];
						printf("%s", flag?"●":"○");
					}
				}
				printf("\n");
			}
			printf("uchar code key[32] = {");
			for(k=0; k<31; k++){
				printf("0x%02X,", buffer[k]);
			}
			printf("0x%02X};\n", buffer[31]);
			printf("\n");
		}
	}
	fclose(fphzk);
	fphzk = NULL;
	return 0;
}
