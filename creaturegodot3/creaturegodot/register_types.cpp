/* register_types.cpp */

#include "register_types.h"
#include "creaturegodot.h"

void register_creaturegodot_types() {

        ClassDB::register_class<CreatureGodot>();
}

void unregister_creaturegodot_types() {
   //nothing to do here
}