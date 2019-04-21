///*
//This code primarily comes from
//http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
//and
//http://www.binarii.com/files/papers/c_sockets.txt
// */
#include<stdio.h>
#include "temp_calc.h"

int n = 1;
int count = 0; //number of readings

temperature my_temp = {-274, -274, -274};

static double temps[3600];

void reset(){
  my_temp.avg = temps[0];
  my_temp.low = temps[0];
  my_temp.high = temps[0];
  n = 1;
}

double update_temp(double reading){
  int index = count % 3600;
  temps[index] = reading;
  count++;
  return 0;
}

temperature get_temp() {
  reset();
  if(count < 3600){
    for(int i=0;i<count;i++){
      if(temps[i] < my_temp.low){
        my_temp.low = temps[i];
      }
      if(temps[i] > my_temp.high){
        my_temp.high = temps[i];
      }
      my_temp.avg= (my_temp.avg * (n-1) + temps[i]) / n;
      n++;
    }
  }else{
    for(int i=0;i<3600;i++){
      if(temps[i] < my_temp.low){
        my_temp.low = temps[i];
      }
      if(temps[i] > my_temp.high){
        my_temp.high = temps[i];
      }
      my_temp.avg= (my_temp.avg * (n-1) + temps[i]) / n;
      n++;
    }
  }
  return my_temp;
}
