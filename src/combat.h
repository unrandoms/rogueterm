#pragma once
#include <string>

struct Entity;

struct CombatResult {
    int  damage;
    bool killed;
    std::string message;
};

// Resolve one attack: attacker hits defender.
// damage = max(1, attack + roll(1..3) - defense)
CombatResult resolve_attack(Entity& attacker, Entity& defender);
