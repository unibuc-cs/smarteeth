#pragma once

#include <utility>

enum LedsColor //culorile pe care le ia ledul pentru a stabili in ce directie urmeaza sa mearga utilizatorul
{
    Grey = 1, //nu este aprins, utilizatorul a terminat perierea indicata de leduri
    Blue,     //dreapta
    Purple,   //stanga
    Red       //stop - in cazul sangerarilor, se opreste pentru clatire
};

//pp ca sunt perioati cate 2 dinti concomitent
enum Direction
{
    No_Direction = 1,
    Right,
    Left,
    Turn_Off
};

// Intoarce directia care trebuie afisata pe leduri si culoarea lor.
std::pair<LedsColor, Direction> getDirection();
