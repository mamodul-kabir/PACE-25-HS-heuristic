#include <string>
#include <vector>

using namespace std; 

template <typename T>
float to_float(T value);

void loadInp(); 
void generate_csv(double t_limit); 				//this is used to generate datasets from the public instances. 
std::vector<float> get_vector(int r);
void do_mapping(); 
void mergeFromNuSC(); 
void printResult(); 
void init(); 