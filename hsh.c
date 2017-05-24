#include "cmdscan.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#define BUFSIZE 1024

//int cmdscan(char * cmdbuf, struct cmd * com);

int main(){
	char buf[BUFSIZE];
	struct cmd command;
	pid_t pid1, pid2;
	int fd[2],fdout, status, n, fdin;

	printf(">");	
	while(fgets(buf, BUFSIZE, stdin)!=NULL){
		if(cmdscan(buf, &command) == -1){
			printf("illegal format\n");
			continue;
		}
		if (strcmp(buf, "exit") == 0){
			exit(0);
		}
		else{	
        		if (command.piping == 0){
				if(command.redirect_in == 1){
                                	fdin = open(command.infile, O_RDONLY);
                                }
				if(command.redirect_out == 1 || (command.redirect_out ==1 && command.redirect_append==1)){
					if(command.redirect_append == 0){
						fdout = open(command.outfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
					}
					else{
						fdout = open(command.outfile, O_WRONLY| O_APPEND |O_CREAT, 0644);
					}
					if (fdout == -1){
						perror("open error");
						return -1;
					}
					if(fdin == -1){
						perror("open error");
						return -1;
					}
					if((pid1 = fork()) == -1){
        	                                perror("fork error");
                	                        exit(-1);
	                                }                               
                        	        else if(pid1 == 0){
						if(command.redirect_in == 1){
							dup2(fdin, STDIN_FILENO);
							close(fdin);
						}
						dup2(fdout, STDOUT_FILENO); 
                                        	if (execvp(command.argv1[0], command.argv1) < 0){
                                                	perror("exec 1 error");
                                                	exit(-1);
                                        	}
                                        	exit(0);
                                	}
                                	else{
						close(fdout);
						if(command.background == 1){
							wait(NULL);
							printf(">");
						}
						else if(command.background == 0){
							while(wait(&status) !=pid1);
							printf(">");
						}
                                	}

				}
				else{
					if((pid1 = fork()) == -1){
						perror("fork error");
						exit(-1);
					}
					else if(pid1 == 0){ 
						if (command.redirect_in == 1){
							dup2(fdin, STDIN_FILENO);
							close(fdin);
						}
						if (execvp(command.argv1[0], command.argv1) < 0){
                        				perror("exec 1 error");
                        				exit(-1);
                				}
						exit(0);
					}
					else{
						if(command.background ==1){
							wait(NULL);
							printf(">");
						}
						else if(command.background ==0){
							while(wait(&status) != pid2);
							printf(">");
						}
					}
       				}}

			else if(command.piping == 1 && command.redirect_in == 1 && command.redirect_out == 0 && command.redirect_append == 0){
                                int fdin = open(command.infile, O_RDONLY);
				if (fdin < 0){
					perror("open error");
					return -1;
				}
				if (pipe(fd) < 0){
                                        perror("pipe error");
                                        return -1;
                                }
                                pid1 = fork();
                                if (pid1 < 0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid1 == 0) {
					dup2(fdin, STDIN_FILENO);
                                        close(fd[0]);
                                        dup2(fd[1], STDOUT_FILENO);
                                        close(fd[1]);
                                        execvp(command.argv1[0],command.argv1);
                                        perror("exec error");
                                        return -1;
                                }
                                pid2 = fork();
                                if (pid2<0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid2==0) {
                                        close(fd[1]);
                                        dup2(fd[0], STDIN_FILENO);
                                        close(fd[0]);
                                        execvp(command.argv2[0],command.argv2);
                                        perror("exec error");
                                        return -1;
                                }

                                close(fd[0]);
                                close(fd[1]);
				close(fdin);
				if(command.background == 1){
					wait(NULL);
					printf(">");
				}
				else if(command.background == 0){
                                	waitpid(pid1,NULL,0);
                                	waitpid(pid2,NULL,0);
                                	printf(">");
				}
                        }

				
			else if(command.piping == 1 && command.redirect_in == 0 && command.redirect_out == 0 && command.redirect_append == 0){
				if (pipe(fd) < 0){
					perror("pipe error");
					return -1;
				}
				pid1 = fork();
				if (pid1 < 0) {
					perror("fork error");
					return -1;
				}
				if (pid1 == 0) {
					close(fd[0]);
					dup2(fd[1], STDOUT_FILENO);
					close(fd[1]);
					execvp(command.argv1[0],command.argv1);
					perror("exec error");
					return -1;
				}
				pid2 = fork();
				if (pid2<0) {
					perror("fork error");
					return -1;
				}
				if (pid2==0) {
					close(fd[1]);
					dup2(fd[0], STDIN_FILENO);
					close(fd[0]);
					execvp(command.argv2[0],command.argv2);
					perror("exec error");
					return -1;
				}

				close(fd[0]);
				close(fd[1]);
				if(command.background == 1){
					wait(NULL);
					printf(">");
				}
				if(command.background == 0){
					waitpid(pid1,NULL,0);
					waitpid(pid2,NULL,0);
					printf(">");
				}					
			}

			else if(command.piping == 1 && command.redirect_in == 0 && command.redirect_out == 1 && command.redirect_append == 0){
				int fdout, fd2[2], n;
				if((fdout = open(command.outfile, O_WRONLY| O_TRUNC | O_CREAT, 0644)) == -1){
					perror("open error");
					return -1;
				}
                                if (pipe(fd) < 0){
                                        perror("pipe error");
                                        return -1;
                                }
				if (pipe(fd2) < 0){
                                        perror("pipe error");
                                        return -1;
                                }

                                pid1 = fork();
                                if (pid1 < 0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid1 == 0) {
					close(fd2[0]);
					close(fd2[1]);
                                        close(fd[0]);
                                        dup2(fd[1], STDOUT_FILENO);
                                        close(fd[1]);
                                        execvp(command.argv1[0],command.argv1);
                                        perror("exec error");
                                        return -1;
                                }
                                pid2 = fork();
                                if (pid2<0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid2==0) {
					close(fd2[0]);
                                        close(fd[1]);
                                        dup2(fd[0], STDIN_FILENO);
					dup2(fd2[1], STDOUT_FILENO);
					close(fd2[1]);
                                        close(fd[0]);
                                        execvp(command.argv2[0],command.argv2);
                                        perror("exec error");
                                        return -1;
                                }
	
                                close(fd[0]);
                                close(fd[1]);
				close(fd2[1]);
                                close(fdout);
				while((n = read(fd2[0], &buf, BUFSIZE)) != 0){
					if((write(fdout, buf, n)) != n){
						perror("write error");
						return -1;
					}
				}
				close(fd2[0]);
				if(command.background == 0){
					waitpid(pid1, NULL, 0);
					waitpid(pid2, NULL, 0);
                                	printf(">");
				}
				else if(command.background == 1){
					wait(NULL);
					printf(">");
				}
                        }

		else if(command.piping == 1 && command.redirect_in  == 1 && command.redirect_out == 1 && command.redirect_append == 0){
			 	int fdout,fdin, fd2[2], n;
                                if((fdout = open(command.outfile, O_WRONLY| O_TRUNC | O_CREAT, 0644)) == -1){
                                        perror("open error");
                                        return -1;
                                }
				if((fdin = open(command.infile, O_RDONLY)) == -1){
					perror("open error");
					return -1;
				}
                                if (pipe(fd) < 0){
                                        perror("pipe error");
                                        return -1;
                                }
                                if (pipe(fd2) < 0){
                                        perror("pipe error");
                                        return -1;
                                }

                                pid1 = fork();
                                if (pid1 < 0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid1 == 0) {
                                        close(fd2[0]);
                                        close(fd2[1]);
					dup2(fdin, STDIN_FILENO);
                                        close(fd[0]);
                                        dup2(fd[1], STDOUT_FILENO);
                                        close(fd[1]);
                                        execvp(command.argv1[0],command.argv1);
                                        perror("exec error");
                                        return -1;
                                }
                                pid2 = fork();
                                if (pid2<0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid2==0) {
                                        close(fd2[0]);
                                        close(fd[1]);
                                        dup2(fd[0], STDIN_FILENO);
                                        dup2(fdout, STDOUT_FILENO);
                                        close(fd2[1]);
                                        close(fd[0]);
                                        execvp(command.argv2[0],command.argv2);
                                        perror("exec error");
                                        return -1;
                                }

                                close(fd[0]);
                                close(fd[1]);
                                close(fd2[1]);
				close(fdout);
				close(fdin);
                                while((n = read(fd2[0], &buf, BUFSIZE)) != 0){
                                        if((write(fdout, buf, n)) != n){
                                                perror("write error");
                                                return -1;
                                        }
				}
				close(fd2[0]);
				if(command.background == 0){
                                	waitpid(pid1, NULL, 0);
					waitpid(pid2, NULL, 0);
                                	printf(">");
				}
				else if(command.background == 1){
					wait(NULL);
					printf(">");
				}
		}
		
		else if(command.piping == 1 && command.redirect_in == 0 && command.redirect_out == 1 && command.redirect_append == 1){
                                printf("MADE IT\n");
				int fdappend, fd3[2], n;
                                if((fdappend = open(command.outfile, O_WRONLY | O_CREAT, 0644)) == -1){
                                        perror("open error");
                                        return -1;
                                }
                                if (pipe(fd) < 0){
                                        perror("pipe error");
                                        return -1;
                                }
                                if (pipe(fd3) < 0){
                                        perror("pipe error");
                                        return -1;
                                }

                                pid1 = fork();
                                if (pid1 < 0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid1 == 0) {
                                        close(fd3[0]);
                                        close(fd3[1]);
                                        close(fd[0]);
                                        dup2(fd[1], STDOUT_FILENO);
                                        close(fd[1]);
                                        execvp(command.argv1[0],command.argv1);
                                        perror("exec error");
                                        return -1;
                                }
                                pid2 = fork();
                                if (pid2<0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid2==0) {
                                        close(fd3[0]);
                                        close(fd[1]);
                                        dup2(fd[0], STDIN_FILENO);
                                        dup2(fd3[1], STDOUT_FILENO);
                                        close(fd3[1]);
                                        close(fd[0]);
                                        execvp(command.argv2[0],command.argv2);
                                        perror("exec error");
                                        return -1;
                                }

                                close(fd[0]);
                                close(fd[1]);
                                close(fd3[1]);
                                close(fdappend);
				lseek(fdappend, 0 ,SEEK_END);
                                while((n = read(fd3[0], &buf, BUFSIZE)) != 0){
                                        if((write(fdappend, buf, n)) != n){
                                                perror("write error");
                                                return -1;
                                        }
                                }
                                close(fd3[0]);
                            	if(command.background == 0){	
					waitpid(pid1, NULL, 0);
					waitpid(pid2, NULL, 0);
                                	printf(">");
				}
				else if(command.background == 1){
					wait(NULL);
					printf(">");
				}
                        }
	
		else if(command.piping == 1 && command.redirect_in == 1 && command.redirect_out == 1 && command.redirect_append == 1){
                                int fdappend, fd3[2], n, fdin;
                                if((fdappend = open(command.outfile, O_WRONLY | O_CREAT, 0644)) == -1){
                                        perror("open error");
                                        return -1;
                                }
				if ((fdin = open(command.infile, O_RDONLY)) == -1){
					perror("open error");
					return -1;
				}
                                if (pipe(fd) < 0){
                                        perror("pipe error");
                                        return -1;
                                }
                                if (pipe(fd3) < 0){
                                        perror("pipe error");
                                        return -1;
                                }

                                pid1 = fork();
                                if (pid1 < 0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid1 == 0) {
                                        close(fd3[0]);
                                        close(fd3[1]);
					dup2(fdin, STDIN_FILENO);
                                        close(fd[0]);
                                        dup2(fd[1], STDOUT_FILENO);
                                        close(fd[1]);
                                        execvp(command.argv1[0],command.argv1);
                                        perror("exec error");
                                        return -1;
                                }
                                pid2 = fork();
                                if (pid2<0) {
                                        perror("fork error");
                                        return -1;
                                }
                                if (pid2==0) {
                                        close(fd3[0]);
                                        close(fd[1]);
                                        dup2(fd[0], STDIN_FILENO);
                                        dup2(fd3[1], STDOUT_FILENO);
                                        close(fd3[1]);
                                        close(fd[0]);
                                        execvp(command.argv2[0],command.argv2);
                                        perror("exec error");
                                        return -1;
                                }

                                close(fd[0]);
                                close(fd[1]);
                                close(fd3[1]);
                                lseek(fdappend, 0 ,SEEK_END);
                                while((n = read(fd3[0], &buf, BUFSIZE)) != 0){
                                        if((write(fdappend, buf, n)) != n){
                                                perror("write error");
                                                return -1;
                                        }
                                }
                                close(fd3[0]);
                                close(fdappend);
				close(fdin);
                                if(command.background == 0){
					waitpid(pid1, NULL, 0);
					waitpid(pid2, NULL, 0);
					printf(">");	
				}
				if(command.background == 1){
					wait(NULL);
					printf(">");
				}
			}
		}
	
	}
	return 0;
}
