import pandas as pd
import matplotlib.pyplot as plt

def plot_csv_graph(file_path, num_lines):
    # Lire le fichier CSV
    try:
        data = pd.read_csv(file_path)
    except FileNotFoundError:
        print(f"Le fichier {file_path} n'a pas été trouvé.")
        return
    except pd.errors.EmptyDataError:
        print("Le fichier est vide.")
        return
    except pd.errors.ParserError:
        print("Erreur de parsing du fichier CSV.")
        return

    # Vérifier si les colonnes nécessaires existent
    if 'Backend_Time' not in data.columns or 'ph' not in data.columns:
        print("Les colonnes 'Backend_Time' et 'ph' doivent être présentes dans le fichier CSV.")
        return

    # Sélectionner les dernières lignes du DataFrame
    if num_lines > len(data):
        print(f"Le nombre de lignes demandé ({num_lines}) dépasse le nombre de lignes disponibles ({len(data)}).")
        num_lines = len(data)
    
    data_subset = data.tail(num_lines)

    # Créer le graphique
    plt.figure(figsize=(10, 5))
    plt.plot(data_subset['Backend_Time'], data_subset['ph'], marker='o', linestyle='-', color='b')
    plt.xlabel('Backend_Time')
    plt.ylabel('ph')
    plt.title('Graphique de Backend_Time vs ph')
    plt.grid(True)
    plt.tight_layout()

    # Afficher le graphique
    plt.show()

# Spécifiez le chemin du fichier CSV et le nombre de lignes à partir de la fin
file_path = r'R:\Bioreactor\ServerFastAPI\data\data.csv'
num_lines = int(input("Entrez le nombre de lignes à partir de la fin à afficher : "))

plot_csv_graph(file_path, num_lines)
