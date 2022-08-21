# fast_format
- Small, fast formating header based util in c++ : string to integer/double , integer/double to string fast conversion library 

- specifying precision in digits after decimal, for double to string conversion - lesser needed, faster it is. 

```
  // only 3 digits precison will be preserved in the double to string conversion
  Format<double, 3> format; 
```

#### API
1. toDouble(const char* str)
2. toInteger(const char* str)
3. toString(int value)
4. toString(double value)

###### Use cases 
```
1. char* value = "7.23"; 
   std::cout << toDouble(value) << '\n';
   
2. char* value = "723"; 
   std::cout << toInteger(value) << '\n';
   
3. double value = 7.23; 
   Format<double> format; // default 9 digits precison after decimal 
   format.convert(value); 
   std::cout << format.toString() << '\n';
  
4. int value = 723; 
   Format<int> format; 
   format.convert(value); 
   std::cout << format.toString() << '\n'; 
```
