/*
Author: Hieu Doan
Date: 08/03/2021
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

char* userinput(); //getting user input 
int sizeofline(char *, int); //How many character per lines
int numlines(char *, int);  //How many line in a file
void setlines(int *, char *, int); //set lines in array
void ncommand(int *, char *, int); //do the n command
void pcommand(int *, char *, int); //do the p command
void dcommand(int *, char *, int, int); //do the d command
void mcommand(int *, char *, int, int, int); //do the m command
void icommand(int *, char *, int, int); //do the i command
void acommand(int *, char *, int, int); //do the a command
void wcommand(int *, char * ,int, int, char *); //do the w command

enum{
	MAXINPUT = 128
};


int
main(int argc, char *argv[])
{
	int size, used, fd, n, i, currLine, dest, temp;
	struct stat ccstat;
	char *buf, *p, *ptr, *user;
	char choice[5];
	int *lines;
	if(argc != 2) {
		fprintf(stderr, "Usage: te name\n");
		exit(1);
	}
	fd = open(argv[1], O_RDONLY);
	if(fd < 0) {
		if(errno == ENOENT) {
			fd = open(argv[1], O_CREAT | O_RDWR);
			if (fd < 0) {
				perror("open");
				exit(2);
			}
		}
	}
	if (stat(argv[1], &ccstat) < 0){
		perror("stat");
		exit(3);
	}
	size = ccstat.st_size;
	buf = malloc(size + 1);
	read(fd, buf, size);
	buf[size] = '\0';
	n = numlines(buf, size);
	lines = malloc(n * sizeof(int *));
	setlines(lines, buf, size);
	currLine= 1;
	do{
		p = buf;
		printf("%.*s", sizeofline(p, lines[currLine - 1]), p + lines[currLine - 1]);
		printf(":");
		fgets(choice, 5, stdin); 
		if(choice[0] == 'p')
			pcommand(lines, buf, n);
		else if(choice[0] == 'n')
			ncommand(lines, buf, n);
		else if(choice[0] == 'q')
			printf("Good Bye\n");
		else if(atoi(choice) != 0){
			currLine = atoi(choice);
			if (currLine > n){
				currLine = n;
			}
		}
		else if(choice[0] == 'd'){

			dcommand(lines, buf, n, currLine);
			if (currLine == n){
				currLine = n - 1;
			}
			n -= 1;
		}
		else if(choice[0] == 'm'){
			dest = strtol(choice + 1, &ptr, 10);
			if(dest != 0) {
				mcommand(lines, buf, n, currLine, dest);
				currLine = dest - 1;
			}
			else if (choice[1] == '$'){
				dest = n + 1;
				mcommand(lines, buf, n, currLine, dest);
				currLine = dest - 1;
			}
			else
				printf("Invalid range");
		}
		else if(choice[0] == 'i'){
			temp = currLine;
			do{
				user = userinput();
				if(strcmp(user, ".EOF") != 0){
					buf = realloc(buf, strlen(buf) + strlen(user) + 1);
					strcat(buf, user);
					n += 1;
					lines = realloc(lines, n * sizeof(int *));
					lines[n - 1] = strlen(buf) - strlen(user);
					icommand(lines, buf, n, temp);
					temp += 1;
					free(user);
				}
			}while(strcmp(user, ".EOF") != 0);
		}
		else if(choice[0] == 'a'){
			temp = currLine;
			do{
				user = userinput();
				if(strcmp(user, ".EOF") != 0){
					buf = realloc(buf, strlen(buf) + strlen(user) + 1);
					strcat(buf, user);
					n += 1;
					lines = realloc(lines, n * sizeof(int *));
					lines[n - 1] = strlen(buf) - strlen(user);
					acommand(lines, buf, n, temp);
					temp += 1;
					free(user);
				}
			}while(strcmp(user, ".EOF") != 0);
		}
		else if(choice[0] == 'w'){
			wcommand(lines, buf, n, fd, argv[1]);
		}
		else
			printf("Not an editor command\n");
		printf("\n");
	} while(choice[0] != 'q');

	free(buf);
	free(lines);
	close(fd);
	exit(0);
	
}

void
setlines(int *lines, char *buf, int nchar)
{
	int i, n;
	char *p;
	n = 1;
	lines[0] = 0;
	for (i = 0, p = buf; i < nchar; i++, p++){
		if(*p == '\n')
			lines[n++] = i + 1;
	}
}

int
numlines(char *buf, int nchar)
{
	int i, nline;
	char *p;
	p = buf;
	nline = 0;
	for (i = 0; i < nchar; i++){
		if(*p == '\n')
			nline++;
		p++;
	}
	return nline;
}

int
sizeofline(char *p, int pos)
{
	if (strlen(p) < 1)
		return 0;
	int size;
	size = 0;
	p += pos;
	while (p[size] != '\n')
		size++;
	return size + 1;
}

void
pcommand(int *lines, char *buf, int n)
{
	int i;
	char *p;
	p = buf;
	for(i = 0; i < n; i++){
		printf("%.*s", sizeofline(p, lines[i]), p + lines[i]);	
	}
}

void
ncommand(int *lines, char *buf, int n)
{
	int i;
	char *p;
	p = buf;
	for(i = 0; i < n; i++){
		printf("%d. %.*s", i+1, sizeofline(p, lines[i]), p + lines[i]);
	}
}

void
dcommand(int *lines, char *buf, int n, int currLine)
{
	int i;
	char *p;
	p = buf;
	memmove(lines + (currLine - 1), lines + currLine, (n - currLine)*sizeof(*lines));
	lines = realloc(lines, (n - 1) * sizeof(int *));
}

void
mcommand(int *lines, char *buf, int n, int currLine, int dest)
{
	int temp;
	temp = lines[currLine - 1];
	if (currLine < dest) {
		memmove(lines + (currLine - 1), lines + currLine, (dest - currLine - 1) * sizeof(*lines));
		lines[dest - 2] = temp;
	}
	else if(currLine >  dest) {
		memmove(lines + (dest - 1), lines + (dest - 2), (currLine - (dest - 1)) * sizeof(*lines));
		lines[dest - 2] = temp;
	}
	else {
		lines[currLine - 1] = temp;
	}
}

char *userinput(){
	char *input, *p;
	int count;
	input = malloc(MAXINPUT);
	fgets(input, MAXINPUT, stdin);
	count = 0;
	for (p = input; *p != '\0'; p++)
	{
		if (*p != '.') 
			count++;					    
		if ((*p == '\n') && strchr(input, '.'))
			count--;
	}
	if (count < 1)
		return ".EOF";
	return input;
}

void
icommand(int *lines, char *buf, int n, int currLine)
{
	int temp;
	temp = lines[n - 1];
	memmove(lines + currLine, lines + (currLine - 1), ((n + 1)  - currLine) * sizeof(*lines));
	lines[currLine - 1] = temp;
}

void
acommand(int *lines, char *buf, int n, int currLine)
{
	int temp;
	temp = lines[n - 1];
	memmove(lines + (currLine + 1), lines + (currLine + 1) - 1, ((n + 1)  - (currLine + 1)) * sizeof(*lines));
	lines[currLine] = temp;
}

void
wcommand(int *lines, char *buf, int n, int fd, char *name)
{
	char *p;
	int i;
	p = buf;
	close(fd);
	fd = open(name, O_TRUNC | O_WRONLY);
	if(fd < 0){
		perror("open");
		exit(4);
	}
	for(i = 0; i < n; i++){
   	write(fd, p + lines[i], sizeofline(p, lines[i]));
   }
}

