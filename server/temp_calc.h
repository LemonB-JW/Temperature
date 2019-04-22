typedef struct{
  double avg;
  double low;
  double high;
} temperature;

void reset();
double update_temp(double* reading, int isF);
temperature get_temp();
