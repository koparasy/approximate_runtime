#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define  Num_Threads 4
int result[Num_Threads];
pthread_t threads[Num_Threads];
typedef struct pair{
  unsigned int num;
  unsigned int id;
}p_t;

p_t inputs[Num_Threads];
drand48_data status[MAX_THREADS],*pts[MAX_THREADS];
float randomFloat()
{
  float r = (float)rand()/(float)RAND_MAX;
  return r;
}

void* monte_carlo_pi(void *N){
  p_t *me = (p_t*) N;
  unsigned int Num_samples = me->num;
  printf("%d --- %d \n",Num_samples,me->id);
  int under_curve = 0;
  int count;
  double a,x,y;
  
  for (count=0; count<Num_samples; count++)
  {
    x= 1;//randomFloat();
    y= 1;//randomFloat();
    a= x*x + y*y;
    
    if ( a <= 1.0){
      under_curve ++;
    }
  }
  
  
  result[me->id] = under_curve;
  return;
}


int main(int argc, char *argv){
  int i;
  double res;
  struct timespec  tv1, tv2;
  unsigned int Num_samples = 1000000000;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);  
  for(i = 0; i < Num_Threads ; i++){
    inputs[i].num = Num_samples / Num_Threads;
    inputs[i].id = i;
    pthread_create(&threads[i], NULL, monte_carlo_pi, &inputs[i]);
  }


  for( i = 0 ;  i < Num_Threads; i++)
    pthread_join(threads[i],NULL);
  
  clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);
  
  printf ("Total time = %10g seconds\n",
	  (double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
	  (double) (tv2.tv_sec - tv1.tv_sec));
  
  for( i = 1 ;  i < Num_Threads; i++)
    result[0] += result[i];
  res = ((double) result[0] / Num_samples) * 4.0;  
  printf("PI is : %2.16g \n",res);
  
}