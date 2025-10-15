/*
 * Example usage of sample_gen1_ou_teams.h
 * 
 * This file demonstrates how to use the pre-defined RBY OU teams
 * in the simulator.
 */

#include <stdio.h>
#include "sample_gen1_ou_teams.h"

// Example function to load a team into a player structure
// (This is a simplified example - actual integration would depend on your Player struct)
void print_team_info(int team_index) {
    if (team_index < 0 || team_index >= NUM_SAMPLE_TEAMS) {
        printf("Invalid team index. Must be 0-%d\n", NUM_SAMPLE_TEAMS - 1);
        return;
    }
    
    const Team* team = &SAMPLE_GEN1_OU_TEAMS[team_index];
    
    printf("Team %d:\n", team_index + 1);
    for (int i = 0; i < 6; i++) {
        printf("  Pokemon %d:\n", i + 1);
        printf("    Species ID: %d\n", team->pokemon[i].species);
        printf("    Moves: [");
        for (int j = 0; j < 4; j++) {
            printf("%d", team->pokemon[i].moves[j]);
            if (j < 3) printf(", ");
        }
        printf("]\n");
    }
}

int main() {
    printf("RBY OU Sample Teams\n");
    printf("===================\n");
    printf("Total teams available: %d\n\n", NUM_SAMPLE_TEAMS);
    
    // Print information for all teams
    for (int i = 0; i < NUM_SAMPLE_TEAMS; i++) {
        print_team_info(i);
        printf("\n");
    }
    
    return 0;
}
