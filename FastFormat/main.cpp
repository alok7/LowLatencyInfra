#include <stdio.h>
#include "FastFormat.hpp"

// Example on how to use FastFormat 
int main()
{

   const char* strDouble = "7.23"; 
   double valueDouble = toDouble(strDouble); 
   printf("%f\n", valueDouble); 
   
   const char* strInt = "723"; 
   int valueInt = toInteger(strInt); 
   printf("%d\n", valueInt); 
   
   double DoubleValue = 7.23; 
   Format<double> format; 
   format.convert(DoubleValue);
   printf("%s\n", format.toString()); 
  
   int IntValue = 723; 
   Format<int> format_; 
   format_.convert(IntValue); 
   printf("%s\n", format_.toString()); 

   return 0;
}
