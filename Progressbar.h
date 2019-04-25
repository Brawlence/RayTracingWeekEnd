// Создано в 2019г. Сергей
#include "stdio.h"
using namespace std;


// берет число от 1 до 10 и выводит полосу прогресса.
void progressBar(int progress){
    for(int j = 1; j <= 53; j++){
        cout << "\b";
    }

    cout << "[";

    for(int k = 1; k <= progress*5 ; k++){
        cout << "*";
    } 
    for(int g = 1; g <= (10-progress)*5 ; g++){
        cout << "_";
    }

    
    cout << "]";
    

    for(int h = 1; h<=28; h++){
        
        cout << "\b";
    
    }
    
    cout << progress*10 << "\%";
}