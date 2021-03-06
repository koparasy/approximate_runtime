#include <stdio.h>
#include <stdlib.h>
#include "coordinator.h"
#include "list.h"
#include "task.h"
#include "group.h"


pool_t *pending_tasks;
pool_t *sig_ready_tasks;
pool_t *non_sig_ready_tasks;
pool_t *executing_tasks;
pool_t *finished_tasks;
int debug_flag = 0;
unsigned int total_workers;
info *my_threads;
task_t **assigned_jobs;

pthread_cond_t cord_condition;
pthread_mutex_t cord_lock;

void explicit_sync(void *args);
void* main_acc(void *args);
void finished_task(task_t* task);

void print_trace(int nsig)
{
  printf("print_trace: got signal %d\n", nsig);
  
  void *array[32];
  size_t size;
  char **strings;
  size_t cnt;
  
  
  size = backtrace(array, 32);
  strings = backtrace_symbols(array, size);
  for (cnt=0; cnt < size; ++cnt) {
    fprintf(stderr, "%s\n", strings[cnt]);
  }
  
}

void my_action(int sig, siginfo_t* siginfo, void *context){
  print_trace(sig);
  exit(0);
}

void action(int sig, siginfo_t* siginfo, void *context){
  int i;
  pthread_t my_id = pthread_self();
  
  for ( i = 0 ; i < total_workers ; i++){
    if(my_id == my_threads[i].my_id ){
      if(my_threads[i].flag == 1){
	setcontext(&(my_threads[i].context));
	return ;
    }
      else{
	break;
      }
    }
  }
  return ;
}



void* init_acc(void *args){

  struct sigaction act;
 
  act.sa_sigaction = action;
  act.sa_flags = SA_SIGINFO;
  
  if ( (sigaction(SIGILL,&act,NULL)<0)||
    (sigaction(SIGFPE,&act,NULL)<0)||
    (sigaction(SIGPIPE,&act,NULL)<0)||
    (sigaction(SIGBUS,&act,NULL)<0)||
    (sigaction(SIGUSR1,&act,NULL)<0)||
    (sigaction(SIGUSR2,&act,NULL)<0)||
     (sigaction(SIGSEGV,&act,NULL)<0)||
      (sigaction(SIGSYS,&act,NULL)<0) ) {
      perror("Could not assign signal handlers\n");
      exit(0);
  }
  main_acc(args);
  
  return NULL;
}


void check_sync(){
  exec_on_elem(groups,explicit_sync);
  return;
}

void init_system(unsigned int reliable_workers , unsigned int nonrel_workers){
  
  
  /* Create the corresponing pulls to store the task descriptors */
  int i;
  total_workers = reliable_workers + nonrel_workers;
  if(total_workers == 0){
    printf("Cannot request 0 workers\n Aborting....\n");
    exit(0);
  }
  
  
  pending_tasks = create_pool();
  
  //Store here significant tasks with 
  //unmet dependencies or tasks waiting for resources.
  sig_ready_tasks = create_pool(); 
  
  // Store here non - significant tasks with 
  //unmet dependencies or tasks waiting for resources.
  non_sig_ready_tasks = create_pool(); 
  //Tasks which are executed at the moment.  
  executing_tasks = create_pool();
  // Tasks finished their execution. I store them in 
  //a queue so that we can re-execute the entire group
  // if requested.
  finished_tasks = create_pool();
  

  
  struct sigaction act;
  act.sa_sigaction = my_action;
  act.sa_flags = SA_SIGINFO;
  
  // Creating a signal handler to catch fault related signals
  
  if ( (sigaction(SIGILL,&act,NULL)<0)||
    (sigaction(SIGFPE,&act,NULL)<0)||
    (sigaction(SIGPIPE,&act,NULL)<0)||
    (sigaction(SIGBUS,&act,NULL)<0)||
    (sigaction(SIGUSR1,&act,NULL)<0)||
    (sigaction(SIGUSR2,&act,NULL)<0)||
    (sigaction(SIGSEGV,&act,NULL)<0)||
    (sigaction(SIGSYS,&act,NULL)<0) ) {
      perror("Could not assign signal handlers\n");
      exit(0);
    }
  
  
  
  my_threads = (info*) malloc (sizeof(info)*total_workers);
  
  assigned_jobs = (task_t**) malloc ( sizeof(task_t*) * total_workers);
  
  
  // Initialize runtime information.
  for( i = 0 ; i < total_workers ; i++){
      assigned_jobs[i] = NULL;
      pthread_cond_init(&my_threads[i].cond,NULL);
      pthread_attr_init(&my_threads[i].attributes);
      pthread_attr_setdetachstate(&my_threads[i].attributes,PTHREAD_CREATE_DETACHED);
      my_threads[i].id = i;
      my_threads[i].sanity = NULL;
      my_threads[i].sanity_args = NULL;
      my_threads[i].execution = NULL;
      my_threads[i].execution_args = NULL;
      my_threads[i].work = 0;
      my_threads[i].checked_results = 1;
      if( i > reliable_workers)
	my_threads[i].reliable = 0;
      else
	my_threads[i].reliable = 1;  
      assigned_jobs[i] = NULL;
      pthread_create(&(my_threads[i].my_id), &(my_threads[i].attributes), init_acc, &my_threads[i]);
  }
}