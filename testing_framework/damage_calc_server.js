#!/usr/bin/env node
/**
 * Node.js server for the Smogon damage calculator
 * Provides a simple HTTP API to calculate damage from Python
 */

const http = require('http');
const url = require('url');

// Import the damage calculator
// Note: You'll need to run: npm install @smogon/calc
const {calculate, Generations, Pokemon, Move, Field} = require('@smogon/calc');

const PORT = 3456;

/**
 * Parse and validate the calculation request
 */
function parseRequest(body) {
    const data = JSON.parse(body);
    
    // Default to generation 1 if not specified
    const genNum = data.gen || 1;
    const gen = Generations.get(genNum);
    
    // Create attacker Pokemon
    const attacker = new Pokemon(gen, data.attacker.name, {
        level: data.attacker.level || 100,
        ability: data.attacker.ability,
        item: data.attacker.item,
        nature: data.attacker.nature,
        evs: data.attacker.evs || {},
        ivs: data.attacker.ivs || {},
        boosts: data.attacker.boosts || {},
        status: data.attacker.status,
        teraType: data.attacker.teraType,
        isCriticalHit: data.attacker.isCriticalHit || false,
    });
    
    // Create defender Pokemon
    const defender = new Pokemon(gen, data.defender.name, {
        level: data.defender.level || 100,
        ability: data.defender.ability,
        item: data.defender.item,
        nature: data.defender.nature,
        evs: data.defender.evs || {},
        ivs: data.defender.ivs || {},
        boosts: data.defender.boosts || {},
        status: data.defender.status,
        teraType: data.defender.teraType,
        curHP: data.defender.curHP,
    });
    
    // Create move
    const move = new Move(gen, data.move.name, {
        ability: data.move.ability,
        item: data.move.item,
        species: data.move.species,
        useMax: data.move.useMax || false,
        useZ: data.move.useZ || false,
        isCrit: data.move.isCrit || false,
        hits: data.move.hits,
    });
    
    // Create field (optional)
    let field = new Field();
    if (data.field) {
        field = new Field({
            gameType: data.field.gameType,
            weather: data.field.weather,
            terrain: data.field.terrain,
            isGravity: data.field.isGravity,
            isMagicRoom: data.field.isMagicRoom,
            isWonderRoom: data.field.isWonderRoom,
            attackerSide: data.field.attackerSide || {},
            defenderSide: data.field.defenderSide || {},
        });
    }
    
    return {gen, attacker, defender, move, field};
}

/**
 * Handle incoming HTTP requests
 */
const server = http.createServer((req, res) => {
    // Set CORS headers
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
    res.setHeader('Content-Type', 'application/json');
    
    // Handle OPTIONS for CORS preflight
    if (req.method === 'OPTIONS') {
        res.writeHead(200);
        res.end();
        return;
    }
    
    const parsedUrl = url.parse(req.url, true);
    
    // Health check endpoint
    if (parsedUrl.pathname === '/health' && req.method === 'GET') {
        res.writeHead(200);
        res.end(JSON.stringify({status: 'ok', message: 'Damage calculator server is running'}));
        return;
    }
    
    // Calculate endpoint
    if (parsedUrl.pathname === '/calculate' && req.method === 'POST') {
        let body = '';
        
        req.on('data', chunk => {
            body += chunk.toString();
        });
        
        req.on('end', () => {
            try {
                const {gen, attacker, defender, move, field} = parseRequest(body);
                const result = calculate(gen, attacker, defender, move, field);
                
                res.writeHead(200);
                res.end(JSON.stringify(result));
            } catch (error) {
                console.error('Calculation error:', error);
                res.writeHead(400);
                res.end(JSON.stringify({
                    error: error.message,
                    stack: error.stack,
                }));
            }
        });
        
        return;
    }
    
    // 404 for unknown routes
    res.writeHead(404);
    res.end(JSON.stringify({error: 'Not found'}));
});

server.listen(PORT, () => {
    console.log(`Damage calculator server running on http://localhost:${PORT}`);
    console.log(`Endpoints:`);
    console.log(`  GET  /health     - Health check`);
    console.log(`  POST /calculate  - Calculate damage`);
});

// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('SIGTERM received, shutting down gracefully...');
    server.close(() => {
        console.log('Server closed');
        process.exit(0);
    });
});
