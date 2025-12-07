"""
Python interface for the Smogon damage calculator.

This module provides a Python wrapper around the TypeScript @smogon/calc package.
It communicates with a Node.js server running the damage calculator.
"""

import json
import subprocess
import time
import atexit
import requests
from typing import Optional, Dict, Any, List, Union
from dataclasses import dataclass, asdict, field


@dataclass
class PokemonData:
    """Configuration for a Pokemon in the damage calculation."""
    name: str
    level: int = 100
    ability: Optional[str] = None
    item: Optional[str] = None
    nature: Optional[str] = None
    evs: Dict[str, int] = field(default_factory=dict)
    ivs: Dict[str, int] = field(default_factory=dict)
    boosts: Dict[str, int] = field(default_factory=dict)
    status: Optional[str] = None
    teraType: Optional[str] = None
    curHP: Optional[int] = None
    isCriticalHit: bool = False


@dataclass
class MoveData:
    """Configuration for a move in the damage calculation."""
    name: str
    ability: Optional[str] = None
    item: Optional[str] = None
    species: Optional[str] = None
    useMax: bool = False
    useZ: bool = False
    isCrit: bool = False
    hits: Optional[int] = None


@dataclass
class FieldData:
    """Configuration for the battle field."""
    gameType: Optional[str] = None
    weather: Optional[str] = None
    terrain: Optional[str] = None
    isGravity: bool = False
    isMagicRoom: bool = False
    isWonderRoom: bool = False
    attackerSide: Dict[str, Any] = field(default_factory=dict)
    defenderSide: Dict[str, Any] = field(default_factory=dict)


@dataclass
class DamageResult:
    """Result of a damage calculation - wraps the Result object from @smogon/calc."""
    
    # Store the raw result
    _raw: Dict[str, Any] = field(repr=False)
    
    def __post_init__(self):
        """Allow accessing any property from the raw result."""
        pass
    
    @property
    def damage(self) -> Union[int, List[int], List[List[int]]]:
        """Get damage value(s)."""
        return self._raw.get('damage', 0)
    
    @property
    def description(self) -> str:
        """Get full damage description."""
        # The Result object may have different description methods
        if 'rawDesc' in self._raw:
            # Build basic description from rawDesc
            desc = self._raw.get('rawDesc', {})
            return f"{desc.get('attackerName', '?')} {desc.get('moveName', '?')} vs. {desc.get('defenderName', '?')}"
        return str(self._raw.get('desc', ''))
    
    def min_damage(self) -> int:
        """Get the minimum damage roll."""
        damage = self.damage
        if isinstance(damage, int):
            return damage
        elif isinstance(damage, list):
            if not damage:
                return 0
            if isinstance(damage[0], list):
                return min(min(sublist) if sublist else 0 for sublist in damage)
            return min(damage)
        return 0
    
    def max_damage(self) -> int:
        """Get the maximum damage roll."""
        damage = self.damage
        if isinstance(damage, int):
            return damage
        elif isinstance(damage, list):
            if not damage:
                return 0
            if isinstance(damage[0], list):
                return max(max(sublist) if sublist else 0 for sublist in damage)
            return max(damage)
        return 0
    
    def average_damage(self) -> float:
        """Get the average damage across all rolls."""
        damage = self.damage
        if isinstance(damage, int):
            return float(damage)
        elif isinstance(damage, list):
            if not damage:
                return 0.0
            # Handle nested arrays (multi-hit moves)
            if isinstance(damage[0], list):
                flat = [item for sublist in damage for item in sublist]
                return sum(flat) / len(flat) if flat else 0.0
            else:
                return sum(damage) / len(damage)
        return 0.0
    
    def __getattr__(self, name):
        """Allow accessing any property from the raw result."""
        if name.startswith('_'):
            raise AttributeError(f"'{type(self).__name__}' object has no attribute '{name}'")
        return self._raw.get(name)


class DamageCalculator:
    """
    Python interface to the Smogon damage calculator.
    
    This class manages a Node.js server running the TypeScript damage calculator
    and provides a Python API to interact with it.
    
    Example:
        calc = DamageCalculator()
        calc.start()
        
        attacker = PokemonData(
            name="Gengar",
            item="Choice Specs",
            nature="Timid",
            evs={"spa": 252, "spe": 252},
            boosts={"spa": 1}
        )
        
        defender = PokemonData(
            name="Chansey",
            item="Eviolite",
            nature="Calm",
            evs={"hp": 252, "spd": 252}
        )
        
        move = MoveData(name="Focus Blast")
        
        result = calc.calculate(gen=5, attacker=attacker, defender=defender, move=move)
        print(result.description)
        
        calc.stop()
    """
    
    def __init__(self, port: int = 3456, server_script: Optional[str] = None):
        """
        Initialize the damage calculator.
        
        Args:
            port: Port number for the Node.js server
            server_script: Path to the Node.js server script (auto-detected if None)
        """
        self.port = port
        self.base_url = f"http://localhost:{port}"
        self.process: Optional[subprocess.Popen] = None
        
        # Auto-detect server script location
        if server_script is None:
            import os
            current_dir = os.path.dirname(os.path.abspath(__file__))
            server_script = os.path.join(current_dir, "damage_calc_server.js")
        
        self.server_script = server_script
        
        # Register cleanup on exit
        atexit.register(self.stop)
    
    def start(self, timeout: float = 5.0) -> None:
        """
        Start the Node.js damage calculator server.
        
        Args:
            timeout: Maximum time to wait for server to start (seconds)
        
        Raises:
            RuntimeError: If server fails to start
            FileNotFoundError: If Node.js or server script not found
        """
        import os
        
        if not os.path.exists(self.server_script):
            raise FileNotFoundError(
                f"Server script not found at {self.server_script}. "
                "Make sure the testing_framework is properly set up."
            )
        
        # Check if node_modules exists
        server_dir = os.path.dirname(self.server_script)
        node_modules = os.path.join(server_dir, "node_modules")
        if not os.path.exists(node_modules):
            raise RuntimeError(
                f"node_modules not found in {server_dir}. "
                "Please run 'npm install' in the testing_framework directory first."
            )
        
        print(f"Starting damage calculator server on port {self.port}...")
        
        try:
            self.process = subprocess.Popen(
                ["node", self.server_script],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
        except FileNotFoundError:
            raise FileNotFoundError(
                "Node.js not found. Please install Node.js (version 14 or higher)."
            )
        
        # Wait for server to be ready
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                response = requests.get(f"{self.base_url}/health", timeout=1)
                if response.status_code == 200:
                    print("✓ Damage calculator server started successfully")
                    return
            except requests.exceptions.ConnectionError:
                time.sleep(0.1)
        
        # Server didn't start in time
        self.stop()
        raise RuntimeError(
            f"Server failed to start within {timeout} seconds. "
            "Check that Node.js is installed and the server script is working."
        )
    
    def stop(self) -> None:
        """Stop the Node.js damage calculator server."""
        if self.process:
            print("Stopping damage calculator server...")
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
            self.process = None
    
    def is_running(self) -> bool:
        """Check if the server is running."""
        try:
            response = requests.get(f"{self.base_url}/health", timeout=1)
            return response.status_code == 200
        except:
            return False
    
    def calculate(
        self,
        gen: int,
        attacker: PokemonData,
        defender: PokemonData,
        move: MoveData,
        field: Optional[FieldData] = None
    ) -> DamageResult:
        """
        Calculate damage for a move.
        
        Args:
            gen: Pokemon generation number (1-9)
            attacker: Attacking Pokemon configuration
            defender: Defending Pokemon configuration
            move: Move configuration
            field: Battle field configuration (optional)
        
        Returns:
            DamageResult with calculation results
        
        Raises:
            RuntimeError: If server is not running or calculation fails
        """
        if not self.is_running():
            raise RuntimeError(
                "Server is not running. Call start() first."
            )
        
        # Build request payload
        payload = {
            "gen": gen,
            "attacker": {k: v for k, v in asdict(attacker).items() if v is not None and v != {} and v != False},
            "defender": {k: v for k, v in asdict(defender).items() if v is not None and v != {} and v != False},
            "move": {k: v for k, v in asdict(move).items() if v is not None and v != {} and v != False},
        }
        
        if field is not None:
            payload["field"] = {k: v for k, v in asdict(field).items() if v is not None and v != {} and v != False}
        
        # Make request
        try:
            response = requests.post(
                f"{self.base_url}/calculate",
                json=payload,
                timeout=10
            )
            
            if response.status_code != 200:
                error_data = response.json()
                raise RuntimeError(
                    f"Calculation failed: {error_data.get('error', 'Unknown error')}"
                )
            
            result_data = response.json()
            return DamageResult(_raw=result_data)
            
        except requests.exceptions.Timeout:
            raise RuntimeError("Calculation request timed out")
        except requests.exceptions.ConnectionError:
            raise RuntimeError("Failed to connect to server. Is it running?")
        except Exception as e:
            raise RuntimeError(f"Calculation failed: {str(e)}")
    
    def __enter__(self):
        """Context manager entry."""
        self.start()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit."""
        self.stop()
