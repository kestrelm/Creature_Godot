/* register_types.cpp */

#include "register_types.h"
#include "object_type_db.h"
#include "creaturegodot.h"

void register_creaturegodot_types() {

        ObjectTypeDB::register_type<CreatureGodot>();
}

void unregister_creaturegodot_types() {
   //nothing to do here
}