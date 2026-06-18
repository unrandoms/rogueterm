#include "combat.h"
#include "entity.h"
#include "rng.h"
#include <algorithm>

CombatResult resolve_attack(Entity& attacker, Entity& defender) {
    int roll   = g_rng.range(1, 3);
    int raw    = attacker.attack + roll - defender.defense;
    int damage = std::max(1, raw);

    defender.hp -= damage;
    bool killed = (defender.hp <= 0);
    if (killed) {
        defender.hp    = 0;
        defender.alive = false;
    }

    std::string msg = attacker.name + " hits " + defender.name +
                      " for " + std::to_string(damage) + " damage.";
    if (killed) msg += " " + defender.name + " dies!";

    return { damage, killed, msg };
}
