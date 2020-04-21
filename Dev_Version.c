#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

//Setup para M=N=100
//Altere aqui os valores:
#define Q_START      0.001
#define Q_END        0.01
#define Q_INCR       0.0001
#define K            5
#define N_TRIALS     50

int    **temp;

double    quantum_prob;

int** gen_grid(int m, int n)
{
  int **grid;
  grid= (int**) malloc(m*sizeof(int*));
  int i,j;
  for(i=0;i<m;++i)
  {
    grid[i]=(int*) malloc(n*sizeof(int));
    grid[i][0]=2;
    grid[i][n-1]=2;
    for(j=1;j<n-1;++j)
    {
      grid[i][j]=0;
    }
  }
  for(j=1;j<n-1;++j)
  {
  grid[0][j]=2;
  }
  for(j=1;j<n-1;++j)
  {
    grid[m-1][j]=2;
  }
  return grid;
}

void iterate(int ***input,int m, int n, int quantum_state)
{

  int i,j,k,l;
  int **grid;
  grid=*input;
  int n_alive;
  int a,b;
  for(i=0;i<m;++i)
  {
    for(j=0;j<n;++j)
    {
      n_alive=0;
      if(grid[i][j]==2)
        continue;
      for(k=-1;k<2;++k)
      {
        for(l=-1;l<2;++l)
        {
          if(grid[i+k][j+l]==1)
          {
            ++n_alive;
          }
        }
      }
      n_alive-=grid[i][j];
      if(grid[i][j]==0)
      {
        if(n_alive==3)
        {
          temp[i][j]=1;
        }
        else
        {
          temp[i][j]=0;
        }
      }
      if(grid[i][j]==1)
      {
        if(n_alive==2 || n_alive==3)
        {
          temp[i][j]=1;
        }
        else
        {
          temp[i][j]=0;
        }
      }
      if(quantum_state&&rand()<RAND_MAX*quantum_prob)
      {
        (temp[i][j]==1)?(temp[i][j]=0):(temp[i][j]=1);
      }
    }
  }
  int **xpointer;
  xpointer=grid;
  *input=temp;
  temp=xpointer;
}

int f_cell_count(int **state,int m, int n)
{
  int i,j;
  int count=0;
  for(i=0;i<m;++i)
  {
    for(j=0;j<n;++j)
    {
      if(state[i][j]==1)
      {
        ++count;
      }
    }
  }

  return count;
}

void save_config_file(int **grid,int m,int n,char *filename)
{
  FILE *f1;
  f1=fopen(filename,"wb");
  fwrite(&m,sizeof(int),1,f1);
  fwrite(&n,sizeof(int),1,f1);
  int i;
  for(i=0;i<m;++i)
  {
    fwrite(grid[i],sizeof(int),n,f1);
  }
  fclose(f1);
}

int main(int argc, char **argv)
{
  
  srand(time(NULL));
  temp = gen_grid(100, 100);
  FILE *stat_file;
  stat_file=fopen("Results.txt","wt");
  int x;
  int ** testgrid;
  int i,j,k,a;
  int trigger;
  char filename[50];
  for(quantum_prob = Q_START; quantum_prob < Q_END; quantum_prob+= Q_INCR)
  {
    trigger=(int)(10000*quantum_prob+K*sqrt(10000*quantum_prob*(1-quantum_prob)));
    for(i = 0; i < N_TRIALS; ++i)
    {
      temp = gen_grid(100, 100);
      testgrid = gen_grid(100,100);
      for(j = 0; j < 200000; ++j)
      {
        iterate(&testgrid, 100, 100,1);
        if(f_cell_count(testgrid, 100,100)>=trigger)
        {
          iterate(&testgrid, 100, 100,0);
          if(f_cell_count(testgrid, 100,100))
          {
            sprintf(filename,"Dev_Version_cs_files/prob%.4lf_it%6d",quantum_prob,i);
            save_config_file(testgrid,100,100,filename);
            break;
          }
        }
      }
      printf("%6d %.4lf\n",j, quantum_prob);
      fprintf(stat_file,"%.4lf %6d\n",quantum_prob,j);
      for(a = 0; a < 100; ++a)
      {
        free(testgrid[a]);
      }
  	  free(testgrid);
  	  for(a = 0; a < 100; ++a)
 	  {
        free(temp[a]);
      }
      free(temp);
    }
  }
  fclose(stat_file);
  return 0;
}