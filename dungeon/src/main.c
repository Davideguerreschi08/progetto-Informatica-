#include <stdio.h>

int main(){
    while (eroe->hp > 0 && !partita_vinta) {
    stampa_stato(eroe);          
    printf("> ");
    fgets(input, MAX_INPUT, stdin);
    TipoComando cmd = parse(input, argomento);
    esegui_comando(cmd, argomento, eroe, &partita_vinta);

}