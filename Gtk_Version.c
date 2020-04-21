#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <time.h>
#include <string.h>

#define screen_size 500

GtkWidget *window1,
          *button_read_m ,
          *button_read_n,
          *button_read_prob,
          *zoom_sub1,
          *zoom_sub2,
          *darea1,
          *darea2,
          *darea3,
          *quantum_prob_scale,
          *combo_box_top,
          *cell_count1,
          *cell_count2,
          *cell_count3,
          *cell_count4;

int       **grid,
          **temp,
          M=100,
          N=100,
          cellW=5,
          cellH=5,
          translate_x=0,
          translate_y=0,
          pause_play=-1,
          quantum=-1,
          showB = -1,
          showD = -1,
          showA = -1,
          wWidth,
          wHeight,
          topology = 0,
          iteration = 0;

long int  *plottingDataB,
          *plottingDataD,
          *plottingDataA,
          beingBorn,
          dying;

double    quantum_prob=0.01,
          prob_new_config,
          A_scale=10,
          BD_scale=10;
          
char      living_cells[100],
          dead_cells[100],
          died_cells[100],
          born_cells[100],
          A_zoom_sub[100],
          BD_zoom_sub[100];

int** gen_grid(int m, int n,double prob)
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
      (rand()<prob*RAND_MAX)?(grid[i][j]=1):(grid[i][j]=0);
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

void iterate(int ***input,int m, int n)
{
  iteration=iteration%250;
  beingBorn = 0;
  dying = 0;
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
          if(topology==0)
          {
            a=i+k;
            b=j+l;
          }
          else if(topology==1)
          {
            if(i+k==0)
              a=m-2;
            else if(i+k==m-1)
              a=1;
            else
              a=i+k;
            if(j+l==0)
              b=n-2;
            else if(j+l==n-1)
              b=1;
            else
              b=j+l;
          }
          if(grid[a][b]==1)
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
      if(quantum==1&&(rand()<RAND_MAX*quantum_prob))
      {
        (temp[i][j]==1)?(temp[i][j]=0):(temp[i][j]=1);
      }
      if(grid[i][j]==1 && temp[i][j] == 0)
      	{
      	  ++dying;
      	}
      if(grid[i][j] == 0 && temp[i][j] == 1)
        {
          ++beingBorn;
        }
    }
  }
  *(plottingDataB + iteration) = beingBorn; 
  *(plottingDataD + iteration) = dying;
  ++iteration;
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
  *(plottingDataA+iteration)=count;
  return count;
}

gboolean timerf (gpointer data)
{
  if(pause_play==1)
  {
    iterate(&grid,M,N);
  }
  gtk_widget_queue_draw(data);
  sprintf(living_cells,"%d",f_cell_count(grid,M,N));
  gtk_label_set_text(GTK_LABEL(cell_count1),living_cells);
  sprintf(dead_cells,"%d",(M-2)*(N-2)-f_cell_count(grid,M,N));
  gtk_label_set_text(GTK_LABEL(cell_count2),dead_cells);
  sprintf(born_cells,"%ld",beingBorn);
  gtk_label_set_text(GTK_LABEL(cell_count3),born_cells);
  sprintf(died_cells,"%ld",dying);
  gtk_label_set_text(GTK_LABEL(cell_count4),died_cells);
  sprintf(BD_zoom_sub,"1 grid =%g cells",(20*M*N)/(250*BD_scale));
  gtk_label_set_text(GTK_LABEL(zoom_sub1),BD_zoom_sub);
  sprintf(A_zoom_sub,"1 grid =%g cells",(20*M*N)/(250*A_scale));
  gtk_label_set_text(GTK_LABEL(zoom_sub2),A_zoom_sub);
  quantum_prob=gtk_spin_button_get_value(GTK_SPIN_BUTTON(quantum_prob_scale));
  gchar *s;
  s=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box_top));
  if(s[0]=='0')
  {
    topology=0;
  }
  else if(s[0]=='1')
  {
    topology=1;
  }
  g_free(s);
  gtk_widget_queue_draw(darea2);
  gtk_widget_queue_draw(darea3);
  return TRUE;
}

gboolean render(GtkWidget *widget , cairo_t *cr , gpointer data)
{
  cairo_set_source_rgb(cr,1,1,1);
  cairo_rectangle(cr,0,0,500,500);
  cairo_fill(cr);
  int i,j;
  cairo_translate(cr,translate_x*cellW,translate_y*cellH);
  for(i=0;i<M;++i)
  {
    for(j=0;j<N;++j)
    {  
      if((i+translate_y)*cellH<0||(i+translate_y)*cellH>screen_size||(j+translate_x)*cellW<0||(j+translate_x)*cellW>screen_size)
      {
        continue;
      }
      if(grid[i][j]>=1)
      {
        cairo_rectangle(cr, j*cellW, i*cellH, cellW, cellH);
        if(grid[i][j]>1)
        {
          cairo_set_source_rgb(cr,0,0,0.5);
        }
        else
        {
          cairo_set_source_rgb(cr,0,0,0);
        }
        cairo_fill(cr);
      }
    }
  }
  return FALSE;
}

gboolean renderBDGraphs(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  cairo_set_source_rgb(cr,1,1,1);
  cairo_rectangle(cr,0,0,500,250);
  cairo_fill(cr);
  int i;
  cairo_set_source_rgb(cr,0, 0, 0);
  cairo_set_line_width(cr,4);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, 0,250);
  cairo_move_to(cr, 0, 250);
  cairo_line_to(cr, 500,250);
  cairo_stroke(cr);
  cairo_set_line_width(cr,1);
  for(i = 0; i <= 500; i+=20)
  {
    cairo_move_to(cr, i, 0);
    cairo_line_to(cr, i,250);
  }
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, 500,0);
  for(i=0; i <= 250; i+= 20)
  {
    cairo_move_to(cr, 0, 250-i);
    cairo_line_to(cr, 500,250-i);
  }
  cairo_stroke(cr);
  cairo_set_line_width(cr,2);
  if(showB == 1)
  {
  	cairo_set_source_rgb(cr,0,255,0);
    cairo_move_to(cr, 0, 250 - 250*BD_scale*plottingDataB[0]/(M*N));
    for(i=0; i < 250; ++i )
    {
  	  cairo_line_to(cr, 2*i, 250 - 250*BD_scale*plottingDataB[i]/(M*N));
  	  if(i==iteration-1)
  	  {
  	    cairo_move_to(cr, 2*i, 250 - 250*BD_scale*plottingDataB[i+1]/(M*N));
  	    cairo_stroke(cr);
  	    cairo_set_source_rgb(cr,0,0.5,0);
  	  }
    }
    cairo_stroke(cr);
  }
  if(showD == 1)
  {
    cairo_set_source_rgb(cr,255,0,0);
    cairo_move_to(cr, 0, 250 - 250*BD_scale*plottingDataD[0]/(M*N));
    for(i=0; i < 250; ++i )
    {
  	  cairo_line_to(cr, 2*i, 250 - 250*BD_scale*plottingDataD[i]/(M*N));
  	  if(i==iteration-1)
  	  {
  	    cairo_move_to(cr, 2*i, 250 - 250*BD_scale*plottingDataD[i+1]/(M*N));cairo_stroke(cr); 
  	    cairo_stroke(cr);
  	    cairo_set_source_rgb(cr,0.5,0,0);
  	  }
    }
    cairo_stroke(cr);
  }
  return FALSE;
}

gboolean renderAGraph(GtkWidget *widget, cairo_t *cr, gpointer data)
{ 
  cairo_set_source_rgb(cr,1,1,1);
  cairo_rectangle(cr,0,0,500,250);
  cairo_fill(cr);
  int i;
  cairo_set_source_rgb(cr,0, 0, 0);
  cairo_set_line_width(cr,4);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, 0,250);
  cairo_move_to(cr, 0, 250);
  cairo_line_to(cr, 500,250);
  cairo_stroke(cr);
  cairo_set_line_width(cr,1);
  for(i = 20; i <= 500; i+=20)
    {
    cairo_move_to(cr, i, 0);
    cairo_line_to(cr, i,250);
    }
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, 500,0);
  for(i=20; i <= 250; i+= 20)
  {
    cairo_move_to(cr, 0, 250-i);
    cairo_line_to(cr, 500,250-i);
  }
  cairo_stroke(cr);
  cairo_set_line_width(cr,2);
  if(showA == 1)
  {
  	cairo_set_source_rgb(cr, 0, 0, 255);
  	cairo_move_to(cr, 0, 250 - 250*A_scale*plottingDataA[0]/(M*N));
  	for(i = 1; i < 250; ++i)
  	{
  	  cairo_line_to(cr, 2*i, 250 - 250*A_scale*plottingDataA[i]/(M*N));
  	  if(i==iteration)
  	  {
  	    cairo_move_to(cr, 2*i, 250 - 250*A_scale*plottingDataA[i+1]/(M*N));
  	    cairo_stroke(cr); 
  	    cairo_set_source_rgb(cr,0,0,0.5);
  	  }
  	}
    cairo_stroke(cr);  
  }
  return FALSE;
}

gboolean fpause(GtkWidget *Widget, gpointer data)
{
  pause_play*=-1;
  if(pause_play==1)
  {
    gtk_button_set_label(GTK_BUTTON(Widget),"Pause");
  }
  else
  {
    gtk_button_set_label(GTK_BUTTON(Widget),"Play");
  }
  return FALSE;
}

gboolean frand_config(GtkWidget *Widget, gpointer data)
{
  int i;
  for(i=0;i<M;++i)
  {
    free(grid[i]);
    free(temp[i]);
  }
  free(grid);
  free(temp);
  M = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_read_m));
  N = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_read_n));
  prob_new_config = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_read_prob));
  grid=gen_grid(M,N,prob_new_config);
  temp=gen_grid(M,N,0);
  gtk_widget_queue_draw(data);
  return FALSE;
}

gboolean edit(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  if(event->type!=GDK_BUTTON_PRESS)
  {
    return FALSE;
  }
  int x,y,x_darea,y_darea,i,j;
  x=event->x_root;
  y=event->y_root;
  gdk_window_get_origin (gtk_widget_get_window (data), &x_darea, &y_darea);
  int x_click,y_click;
  x_click = x-x_darea;
  y_click = y-y_darea;
  i=(int)y_click/cellH-translate_y;
  j=(int)x_click/cellW-translate_x;
  if(i<=0||i>=M||j<=0||j>=N)
  {
    return FALSE;
  }
  if(grid[i][j]==2)
  {
    return FALSE;
  }
  if(event->button==1)
  {
    grid[i][j]=1;
  }
  else
  {
    grid[i][j]=0;
  }
  gtk_widget_queue_draw(data);
  return FALSE;
}

gboolean fquantum(GtkWidget *Widget, gpointer data)
{
  quantum*=-1;
  return FALSE;
}

gboolean fAliveToggle(GtkWidget *Widget, gpointer data)
{
	showA*=-1;
	return FALSE;
}

gboolean fBToggle(GtkWidget *Widget, gpointer data)
{
	showB*=-1;
	return FALSE;
}

gboolean fDToggle(GtkWidget *Widget, gpointer data)
{
	showD*=-1;
	return FALSE;
}

void save_config_file(char *filename)
{
  FILE *f1;
  f1=fopen(filename,"wb");
  fwrite(&M,sizeof(int),1,f1);
  fwrite(&N,sizeof(int),1,f1);
  int i;
  for(i=0;i<M;++i)
  {
    fwrite(grid[i],sizeof(int),N,f1);
  }
  fclose(f1);
}

gboolean save_function(GtkWidget *Widget, gpointer data)
{
  GtkWidget *save_dialog;
  gint res;
  GtkFileChooser *save_chooser;
  save_dialog= gtk_file_chooser_dialog_new ("Save File",
                                      GTK_WINDOW(data),
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      ("_Cancel"),
                                      GTK_RESPONSE_CANCEL,
                                      ("_Save"),GTK_RESPONSE_ACCEPT,NULL);
  save_chooser = GTK_FILE_CHOOSER (save_dialog);
  gtk_file_chooser_set_do_overwrite_confirmation (save_chooser, TRUE);
  res = gtk_dialog_run (GTK_DIALOG (save_dialog));
  if (res == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (save_chooser);
    save_config_file(filename);
    g_free (filename);
  }
  gtk_widget_destroy (save_dialog);
  return FALSE;
}

void load_config_file(char *filename)
{
  FILE *f1;
  f1=fopen(filename,"rb");
  int i;
  for(i=0;i<M;++i)
  {
    free(grid[i]);
    free(temp[i]);
  }
  free(grid);
  free(temp);
  fread(&M,sizeof(int),1,f1);
  fread(&N,sizeof(int),1,f1);
  grid=gen_grid(M,N,0);
  temp=gen_grid(M,N,0);
  for(i=0;i<M;++i)
  {
    fread(grid[i],sizeof(int),N,f1);
  }
  fclose(f1);
}

gboolean load_function(GtkWidget *Widget, gpointer data)
{
  GtkWidget *load_dialog;
  gint res;
  load_dialog = gtk_file_chooser_dialog_new ("Open File",
                                      data,
                                      GTK_FILE_CHOOSER_ACTION_OPEN,
                                      ("_Cancel"),
                                      GTK_RESPONSE_CANCEL,
                                      ("_Open"),
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);
  res = gtk_dialog_run(GTK_DIALOG(load_dialog));
  if (res == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (load_dialog);
    filename = gtk_file_chooser_get_filename (chooser);
    load_config_file (filename);
    g_free (filename);
  }
  gtk_widget_destroy (load_dialog);
  return FALSE;
}

gboolean keyboard_control(GtkWidget *widget, GdkEventKey *event, gpointer data)
{  
  if(event->keyval==43)
  {
    cellW+=1;
    cellH+=1;
  }
  if(event->keyval==45)
  {
    cellW-=1;
    cellH-=1;
  }
  if(event->keyval==119)
  {
    translate_y-=5;
  }
  if(event->keyval==115)
  {
    translate_y+=5;
  }
  if(event->keyval==97)
  {
    translate_x-=5;
  }
  if(event->keyval==100)
  {
    translate_x+=5;
  }
  if(event->keyval==114)
  {
    translate_x=0;
    translate_y=0;
    cellH=5;
    cellW=5;
    A_scale=10;
    BD_scale=10;
  }
  return FALSE;
}

gboolean change_top(GtkWidget *Widget, gpointer data)
{
  gchar *s;
  s=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(Widget));
  if(s[0]=='0')
  {
    topology=0;
  }
  else if(s[0]=='1')
  {
    topology=1;
  }
  g_free(s);
  return FALSE;
}

gboolean zoom_f(GtkWidget *Widget, gpointer data)
{
  int *x=data;
  switch (*x)
  {
  case 0:
    A_scale*=2;
    break;
  case 1:
    A_scale/=2;
    break;
  case 2:
    BD_scale*=2;
    break;
  case 3:
    BD_scale/=2;
    break;
  }
  return FALSE;
}

void create_style_from_file (gchar *cssFile)
{
  gsize   bytes_written, bytes_read;
  GError *error = 0;
  gchar  *fileUTF8 ;

  GtkCssProvider *provider = gtk_css_provider_new ();
  GdkDisplay *display = gdk_display_get_default ();
  GdkScreen *screen = gdk_display_get_default_screen (display);

  gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider),
               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  fileUTF8 = g_filename_to_utf8 (cssFile, strlen(cssFile), &bytes_read, &bytes_written, &error);
  
  gtk_css_provider_load_from_path (provider, fileUTF8, NULL);
  
  free (fileUTF8);
  
  g_object_unref (provider);
}

int main(int argc, char **argv)
{
  int i,j;
  plottingDataB = calloc(260,sizeof(long int));
  plottingDataD = calloc(260,sizeof(long int));
  plottingDataA = calloc(260,sizeof(long int));
  srand(time(NULL));
  grid=gen_grid(M,N,0);
  temp=gen_grid(M,N,0);
  
  //GTK
  gtk_init(&argc,&argv);
  GtkWidget *window1;
  
  //window1
  window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(window1), FALSE);
  g_signal_connect(window1,"destroy",G_CALLBACK(gtk_main_quit),NULL);
  
  //hbox1
  GtkWidget *hbox1;
  hbox1=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,3);
  gtk_container_add(GTK_CONTAINER(window1),hbox1);
  
  //vbox1
  GtkWidget *vbox1;
  vbox1=gtk_box_new(GTK_ORIENTATION_VERTICAL,3);
  gtk_box_pack_start(GTK_BOX(hbox1),vbox1,FALSE,TRUE,0);
  
  //frame
  GtkWidget *frame1;
  frame1 = gtk_frame_new ("  Game of Life  ");
  gtk_frame_set_label_align (GTK_FRAME(frame1), 0.5, 0.5);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);
  gtk_box_pack_start (GTK_BOX(vbox1), frame1, FALSE, TRUE, 0);
  
  //darea1
  GtkWidget *darea1;
  darea1=gtk_drawing_area_new();
  gtk_widget_set_size_request(darea1,500,500);
  gtk_container_add(GTK_CONTAINER(frame1),darea1);
  g_signal_connect(darea1,"draw",G_CALLBACK(render),NULL);
  
  //timer
  g_timeout_add(100,timerf,darea1);
  //vbox2
  GtkWidget *vbox2;
  vbox2=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_box_pack_start(GTK_BOX(hbox1),vbox2,FALSE,FALSE,3);
  
  //button_pause
  GtkWidget *button_pause;
  button_pause=gtk_button_new_with_label("Play");
  gtk_widget_set_size_request(button_pause,20,20);
  gtk_box_pack_start(GTK_BOX(vbox2),button_pause,FALSE,FALSE,5);
  g_signal_connect(button_pause,"clicked",G_CALLBACK(fpause),NULL);
  
  //new random config
  GtkWidget *label_spin_0, *button_new_rand_config;
  label_spin_0 = gtk_label_new("--------------");
  gtk_box_pack_start(GTK_BOX(vbox2),label_spin_0,FALSE,FALSE,5);
  label_spin_0 = gtk_label_new("Choose M");
  gtk_box_pack_start(GTK_BOX(vbox2),label_spin_0,FALSE,FALSE,5);
  GtkAdjustment *adj1,*adj2,*adj3;
  adj1 = gtk_adjustment_new(100,0,1e5,1,0,0);
  adj2 = gtk_adjustment_new(100,0,1e5,1,0,0);
  adj3 = gtk_adjustment_new(0,0,1,0.01,0,0);
  button_read_m = gtk_spin_button_new(adj1,10,0);
  gtk_box_pack_start(GTK_BOX(vbox2),button_read_m,FALSE,FALSE,5);
  label_spin_0 = gtk_label_new("Choose N");
  gtk_box_pack_start(GTK_BOX(vbox2),label_spin_0,FALSE,FALSE,5);
  button_read_n = gtk_spin_button_new(adj2,10,0);
  gtk_box_pack_start(GTK_BOX(vbox2),button_read_n,FALSE,FALSE,5);
  label_spin_0 = gtk_label_new("Choose probablility");
  gtk_box_pack_start(GTK_BOX(vbox2),label_spin_0,FALSE,FALSE,5);
  button_read_prob = gtk_spin_button_new(adj3,0.1,2);
  gtk_box_pack_start(GTK_BOX(vbox2),button_read_prob,FALSE,FALSE,5);
  button_new_rand_config = gtk_button_new_with_label("Generate new random config");
  gtk_box_pack_start(GTK_BOX(vbox2),button_new_rand_config,FALSE,FALSE,5);
  g_signal_connect(button_new_rand_config,"clicked",G_CALLBACK(frand_config),darea1);
  
  //editing config
  g_signal_connect(window1,"button-press-event",G_CALLBACK(edit),darea1);
  
  //quantum mode
  GtkWidget *quantum_toggle;
  quantum_toggle= gtk_check_button_new_with_label("Enable Quantum Mode");
  gtk_box_pack_start(GTK_BOX(vbox2),quantum_toggle,FALSE,FALSE,5);
  g_signal_connect(quantum_toggle,"toggled",G_CALLBACK(fquantum),NULL);
  label_spin_0 = gtk_label_new("Choose quantum probability");
  gtk_box_pack_start(GTK_BOX(vbox2),label_spin_0,FALSE,FALSE,5);
  GtkAdjustment *adj4;
  adj4 = gtk_adjustment_new(0,0,1,0.0001,0,0);
  quantum_prob_scale = gtk_spin_button_new(adj4,0.01,4);
  gtk_box_pack_start(GTK_BOX(vbox2),quantum_prob_scale,FALSE,FALSE,5);
  
  //graphics workplace
  GtkWidget *vbox3;
  vbox3=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_box_pack_start(GTK_BOX(hbox1),vbox3,FALSE,FALSE,3);

  //living cell count
  GtkWidget *cell_count0, *cell_count_hbox;
  cell_count0=gtk_label_new("Alive - Dead - Were Born - Died");
  gtk_box_pack_start(GTK_BOX(vbox3),cell_count0,FALSE,FALSE,5);
  cell_count_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
  gtk_box_set_homogeneous(GTK_BOX(cell_count_hbox),TRUE);
  gtk_box_pack_start(GTK_BOX(vbox3),cell_count_hbox,FALSE,FALSE,5);
  sprintf(living_cells,"%d",f_cell_count(grid,M,N));
  sprintf(dead_cells,"%d",M*N-f_cell_count(grid,M,N));
  cell_count1 = gtk_label_new(living_cells);
  cell_count2 = gtk_label_new(dead_cells);
  cell_count3 = gtk_label_new("0");
  cell_count4 = gtk_label_new("0");
  gtk_box_pack_start(GTK_BOX(cell_count_hbox),cell_count1,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(cell_count_hbox),cell_count2,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(cell_count_hbox),cell_count3,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(cell_count_hbox),cell_count4,FALSE,FALSE,0);

  //darea2
  darea2=gtk_drawing_area_new();
  gtk_widget_set_size_request(darea2, 500, 250);
  gtk_box_pack_start(GTK_BOX(vbox3),darea2, FALSE, FALSE, 5);
  g_signal_connect(darea2,"draw",G_CALLBACK(renderBDGraphs),NULL);

  //darea3
  darea3 = gtk_drawing_area_new();
  gtk_widget_set_size_request(darea3, 500, 250);
  gtk_box_pack_start(GTK_BOX(vbox3),darea3, FALSE, FALSE, 5);
  g_signal_connect(darea3,"draw",G_CALLBACK(renderAGraph),NULL);

  //buttons for graphics
  GtkWidget *zoom_box1,*zoom_box2,*zoomBD_plus,*zoomBD_minus,*zoomA_plus,*zoomA_minus;
  GtkWidget *vbox4;
  vbox4=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_box_pack_start(GTK_BOX(hbox1),vbox4,FALSE,FALSE,3);
  GtkWidget *BToggle;
  BToggle = gtk_check_button_new_with_label("Show cells being born");
  gtk_box_pack_start(GTK_BOX(vbox4), BToggle, FALSE, FALSE, 100);
  g_signal_connect(BToggle,"toggled",G_CALLBACK(fBToggle),NULL);
  GtkWidget *DToggle;
  DToggle = gtk_check_button_new_with_label("Show cells dying");
  gtk_box_pack_start(GTK_BOX(vbox4), DToggle, FALSE, FALSE, 5);
  
  zoom_box1=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
  gtk_box_pack_start(GTK_BOX(vbox4), zoom_box1, FALSE, FALSE, 5);
  zoomBD_plus=gtk_button_new_with_label("+");
  zoomBD_minus=gtk_button_new_with_label("-");
  gtk_box_pack_start(GTK_BOX(zoom_box1), zoomBD_plus, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(zoom_box1), zoomBD_minus, FALSE, FALSE, 5);
  
  zoom_sub1=gtk_label_new("a");
  gtk_box_pack_start(GTK_BOX(vbox4), zoom_sub1, FALSE, FALSE, 5);
  
  g_signal_connect(DToggle,"toggled",G_CALLBACK(fDToggle),NULL);
  GtkWidget *AToggle;
  AToggle = gtk_check_button_new_with_label("Show living cells");
  gtk_box_pack_start(GTK_BOX(vbox4), AToggle, FALSE, FALSE, 80);
  zoom_box2=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
  gtk_box_pack_start(GTK_BOX(vbox4), zoom_box2, FALSE, FALSE, 5);
  zoomA_plus=gtk_button_new_with_label("+");
  zoomA_minus=gtk_button_new_with_label("-");
  gtk_box_pack_start(GTK_BOX(zoom_box2), zoomA_plus, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(zoom_box2), zoomA_minus, FALSE, FALSE, 5);
  g_signal_connect(AToggle,"toggled",G_CALLBACK(fAliveToggle),NULL);
  int zoom0=0,zoom1=1,zoom2=2,zoom3=3;
  g_signal_connect(zoomA_plus,"clicked",G_CALLBACK(zoom_f),&zoom0);
  g_signal_connect(zoomA_minus,"clicked",G_CALLBACK(zoom_f),&zoom1);
  g_signal_connect(zoomBD_plus,"clicked",G_CALLBACK(zoom_f),&zoom2);
  g_signal_connect(zoomBD_minus,"clicked",G_CALLBACK(zoom_f),&zoom3);
  zoom_sub2=gtk_label_new("a");
  gtk_box_pack_start(GTK_BOX(vbox4), zoom_sub2, FALSE, FALSE, 5);
    
  //save to file
  GtkWidget *button_save;
  button_save = gtk_button_new_with_label("Save current configuration in file");
  gtk_box_pack_start(GTK_BOX(vbox2),button_save,FALSE,FALSE,5);
  g_signal_connect(button_save,"clicked",G_CALLBACK(save_function),window1);
  
  //import from file
  GtkWidget *button_load;
  button_load = gtk_button_new_with_label("Load configuration from file");
  gtk_box_pack_start(GTK_BOX(vbox2),button_load,FALSE,FALSE,5);
  g_signal_connect(button_load,"clicked",G_CALLBACK(load_function),window1);
  
  //keyboard control
  g_signal_connect(window1,"key-press-event",G_CALLBACK(keyboard_control),NULL);
  
  //combobox choose topology
  label_spin_0 = gtk_label_new("Choose frontier conditions:");
  gtk_box_pack_start(GTK_BOX(vbox1),label_spin_0,FALSE,FALSE,3);
  combo_box_top = gtk_combo_box_text_new_with_entry();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box_top),"0 - Rectangle");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box_top),"1 - Torus");
  gtk_box_pack_start(GTK_BOX(vbox1),combo_box_top,FALSE,FALSE,3);
  
  create_style_from_file("css.css");
  //ending
  gtk_window_set_title(GTK_WINDOW(window1),"Game of Life");
  gtk_widget_show_all(window1);
  gtk_main();
  
  for(i=0;i<M;++i)
  {
    free(grid[i]);
    free(temp[i]);
  }
  free(grid);
  free(temp);
  free(plottingDataA);
  free(plottingDataB);
  free(plottingDataD);
  return 0;
}