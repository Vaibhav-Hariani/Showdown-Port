import csv
from pathlib import Path
import requests
from bs4 import BeautifulSoup


def parse_pokemon_attacks(pokedex_number: int, cache_dir: Path) -> list[str]:
    """
    Parses Pokémon attack names from all relevant tables on Serebii.net by looking for
    a header with "Attack Name" or "Move".

    Args:
        pokedex_number (int): The Pokédex number of the Pokémon (e.g., 1 for Bulbasaur).
    """
    # Construct the URL using the given Pokédex number.
    # The Pokédex number is formatted with leading zeros to ensure a 3-digit string.
    url_pokedex_number = str(pokedex_number).zfill(3)
    url = f"https://www.serebii.net/pokedex/{url_pokedex_number}.shtml"

    if cache_dir.joinpath(f"{url_pokedex_number}.html").exists():
        print(f"Loading cached data for: {url}\n")
        with open(
            cache_dir.joinpath(f"{url_pokedex_number}.shtml"), "r", encoding="utf-8"
        ) as f:
            content = f.read()
    else:
        print(f"Attempting to fetch data from: {url}\n")

        # Use a user-agent to mimic a browser, which can help with some sites.
        response = requests.get(url)
        response.raise_for_status()  # Raise an HTTPError for bad status codes

        if cache_dir:
            with open(
                cache_dir.joinpath(f"{url_pokedex_number}.shtml"), "w", encoding="utf-8"
            ) as f:
                f.write(response.text)
        content = response.content

    soup = BeautifulSoup(content, "html.parser")

    # Dictionary to store attacks, categorized by their preceding header
    return [
        a.text.strip()
        for a in soup.find_all("a", href=True)
        if a["href"].startswith("/attackdex-rby") and a["href"].endswith(".shtml")
    ]


if __name__ == "__main__":
    # The Pokédex number for Bulbasaur is 1.
    cache_dir = Path("cache")
    moves_csv = Path("data/pokemon_moves.csv")
    cache_dir.mkdir(exist_ok=True)
    with open(moves_csv, "w", encoding="utf-8") as csvfile:
        fieldnames = ["pokedex_number", "attacks"]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        # TODO: Wrap in a process pool for parallel fetching
        for pokemon_number in range(1, 152):
            attacks = parse_pokemon_attacks(pokemon_number, cache_dir=cache_dir)
            writer.writerow({"pokedex_number": pokemon_number, "attacks": attacks})
