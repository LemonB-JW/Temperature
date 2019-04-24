typedef struct{
  double avg;
  double low;
  double high;
} temperature;

void reset();
double update_temp(double* reading);
temperature get_temp();
