#include "dsh.h"

void seize_tty(pid_t callingprocess_pgid); /* Grab control of the terminal for the calling process pgid.  */
void continue_job(job_t *j); /* resume a stopped job */
void spawn_job(job_t *j, bool fg); /* spawn a new job */

job_t *job_list = NULL; //active list of jobs
/* Sets the process group id for a given job and process */
int set_child_pgid(job_t *j, process_t *p)
{
    if (j->pgid < 0) /* first child: use its pid for job pgid */
        j->pgid = p->pid;
    return(setpgid(p->pid,j->pgid));
}

/* Creates the context for a new child by setting the pid, pgid and tcsetpgrp */
void new_child(job_t *j, process_t *p, bool fg)
{
         /* establish a new process group, and put the child in
          * foreground if requested
          */

         /* Put the process into the process group and give the process
          * group the terminal, if appropriate.  This has to be done both by
          * the dsh and in the individual child processes because of
          * potential race conditions.  
          * */

         p->pid = getpid();

         /* also establish child process group in child to avoid race (if parent has not done it yet). */
         set_child_pgid(j, p);

         if(fg) // if fg is set
		seize_tty(j->pgid); // assign the terminal

         /* Set the handling for job control signals back to the default. */
         signal(SIGTTOU, SIG_DFL);
}




void set_job_status(job_t *j){
	process_t *p;
	pid_t pid;
	for(p = j->first_process; p; p = p->next){
		if(!j->bg){
			while((pid = waitpid(p->pid, &p->status, WNOHANG|WUNTRACED)) == 0);
      }
		else{
      pid = waitpid(p->pid, &p->status, WNOHANG|WUNTRACED);
		}	
    if(pid < 0){
      perror("Process exited abnormally");
      exit(EXIT_FAILURE);
    }
    else{
      if (WIFEXITED(p->status)){
        p->completed = true;
        p->stopped = false;
      }
      else if (WIFSIGNALED(p->status)){
        p->completed = true;
      }
      else if (WIFSTOPPED(p->status)){
        p->stopped = true;
      }
    }
	}
}


/* Spawning a process with job control. fg is true if the 
 * newly-created process is to be placed in the foreground. 
 * (This implicitly puts the calling process in the background, 
 * so watch out for tty I/O after doing this.) pgid is -1 to 
 * create a new job, in which case the returned pid is also the 
 * pgid of the new job.  Else pgid specifies an existing job's 
 * pgid: this feature is used to start the second or 
 * subsequent processes in a pipeline.
 * */

void spawn_job(job_t *j, bool fg) 
{

	pid_t pid;
	process_t *p;
	
	int ppos = 0;
	int fpp[2];
	int bpp[2];
	
	for(p = j->first_process; p; p = p->next) {

	  /* YOUR CODE HERE? */
	  /* Builtin commands are already taken care earlier */
	  if(pipe(fpp) < 0){
		  perror("pipe");
      exit(EXIT_FAILURE);
	  }
	  switch (pid = fork()) {

      case -1: /* fork failure */
        perror("fork");
        exit(EXIT_FAILURE);

      case 0: /* child process  */
        p->pid = getpid();	    
        new_child(j, p, fg);
            
	    /* YOUR CODE HERE?  Child-side code for new process. */

		
			if(p->ofile != NULL){
			  close(STDOUT_FILENO);
			  if(open(p->ofile, O_CREAT|O_WRONLY, S_IRWXU) < 0){
					 exit(EXIT_FAILURE);
			  }
			}
	
			if(p->ifile != NULL){
			  close(STDIN_FILENO);
			  if(open(p->ifile, O_RDONLY, S_IRWXU) < 0){
					exit(EXIT_FAILURE);
			  }
			}
			

			if(p->next != NULL){
			 close(STDOUT_FILENO);
			 dup2(fpp[1],STDOUT_FILENO);
			 close(fpp[0]);
			}
			 
			 if (ppos!=0){
				close(STDIN_FILENO);
				dup2(bpp[0],STDIN_FILENO);
				close(bpp[1]);
			}
			

			execvp(p->argv[0],p->argv);

      perror("\nNew child should have done an exec");
      exit(EXIT_FAILURE);  /* NOT REACHED  */
      break;    /* NOT REACHED */

      default: /* parent */
			  if(pipe(bpp) < 0){
				  perror("pipe");
          exit(EXIT_FAILURE);
			  }
			
        bpp[0] = fpp[0];
        close(bpp[1]);
        close(fpp[1]);
        
        ppos++;
			

        /* establish child process group */
        p->pid = pid;
        set_child_pgid(j, p);
          /* Parent-side code for new process.  */
        if (waitpid(pid, &p->status, WNOHANG|WUNTRACED) < 0){
          perror("fork");
          exit(EXIT_FAILURE);
        }
    }
	}
	/* Parent-side code for new job.*/
	set_job_status(j);
	seize_tty(getpid()); 
	
}



/* Sends SIGCONT signal to wake up the blocked job */
void continue_job(job_t *j) 
{
     if(kill(-j->pgid, SIGCONT) < 0)
          perror("kill(SIGCONT)");
}

job_t* find(pid_t pgid, job_t *starter){
	if(!pgid || !starter) return NULL;
	job_t *cur;
	cur = starter;
	while(cur != NULL) {
		if(cur->pgid == pgid) return cur;
		cur = cur->next;
	}
	return NULL;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 * it immediately.  
 */
bool builtin_cmd(job_t *last_job, int argc, char **argv) 
{
  /* check whether the cmd is a built in command
  */
  if (!strcmp(argv[0], "quit")) {
    exit(EXIT_SUCCESS);
  }

  else if (!strcmp("jobs", argv[0])) {
    job_t *cur;
	  if(job_list->next){
		  cur = job_list->next;
		  while(cur != NULL){
			  if(cur->bg) set_job_status(cur);	
			  cur = cur->next;
		  }	    
		  print_job(job_list->next);

      job_t * cur = job_list;
      job_t* temp;
      while(cur->next!= NULL){
        if(job_is_completed(cur->next)) {
          temp = cur->next->next;
          cur->next = temp;
        }
        else{
          cur = cur->next;
        }
      }
	  }
	  else return true;
  }

	else if (!strcmp("cd", argv[0])) {
		if(chdir(argv[1]) < 0) perror("cd error.");
		return true;
  }
  else if (!strcmp("bg", argv[0])) {
    job_t *j = NULL;
    if(argv[1]){
	    int pgid = (int) (strtol(argv[1], NULL, 0));
	    j = find(pgid, job_list);
		  process_t *p;
      if((j == NULL) || (j->pgid <=0)){
        perror("something went wrong");
      }
      j->bg = true;
      if(!job_is_completed(j)){
        continue_job(j);
        for(p = j->first_process; p; p = p->next){
            p->stopped = false;
            p->status = -1;
        }
      }
    }
	  return true;
  }
  else if (!strcmp("fg", argv[0])) {
	  job_t *j = NULL;
    if(argv[1]){
      int pgid = (int) (strtol(argv[1], NULL, 0));
		  j = find(pgid, job_list);
		  process_t *p;
      if((j == NULL) || (j->pgid <=0)){
        perror("something went wrong");
      }
      if(!job_is_completed(j)){
        continue_job(j);
        seize_tty(j->pgid);
        if(!j->bg);
        else{
          j->bg = false;
        }
        for(p = j->first_process; p; p = p->next){
          p->stopped = false;
        }
        
        set_job_status(j);
        if(job_is_stopped(j)){
          if(job_is_completed(j)){
            delete_job(j, job_list); 
          }
        }
      }
      seize_tty(getpid());
	  }
    return true;
  }
  return false;
}

/* Build prompt messaage */
char* promptmsg() 
{
  char *ret = ((char *) (malloc(sizeof(char)*50)));
	int pid = getpid();
	sprintf(ret, "\ndsh(PID = %d)$ ", pid);
	return ret;
}

int main() 
{

	init_dsh();
	DEBUG("Successfully initialized\n");
	job_list = (job_t *)malloc(sizeof(job_t));
	if(!init_job(job_list)) {
		exit(EXIT_FAILURE);
  }
	job_t *j = (job_t *)malloc(sizeof(job_t));
	if(!init_job(j)) {
		exit(EXIT_FAILURE);
  }
        
	while(1) {
    if(!(j->next = readcmdline(promptmsg()))) {
      if (feof(stdin)) {
        fflush(stdout);
        exit(EXIT_SUCCESS);
        }
    }

		job_t *job1; 
		job_t *cur = j->next;
		while(cur != NULL) {
			if(!builtin_cmd(cur, cur->first_process->argc, cur->first_process->argv)){
				if (cur->bg){
					spawn_job(cur,false);
					job1 = cur;
					cur = cur->next;
          job1->next = NULL;
	        find_last_job(job_list)->next = job1;
        }
				else{
					spawn_job(cur,true);
					if(job_is_stopped(cur)){
						if(job_is_completed(cur)){
							job1 = cur->next;
							delete_job(cur, j);
							cur = job1;
						}
						else{
							job1 = cur;
							cur = cur->next;
              job1->next = NULL;
              find_last_job(job_list)->next = job1;
						}
					}
				}
			}
			else{
				cur = cur->next;
			} 
		}
  }
}