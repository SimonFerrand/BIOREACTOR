import pandas as pd

def load_and_filter_data():
    # Chargement du fichier
    file_path = r"R:\Bioreactor\ServerFastAPI\data\data.csv"
    df = pd.read_csv(file_path, low_memory=False)
    print("\nShape initial du DataFrame:", df.shape)
    
    # Afficher les lignes spécifiques
    print("\nLigne à l'index 1:")
    print(df.iloc[1])
    
    print("\nLigne à l'index 38639:")
    print(df.iloc[38639])
    
    print("\nLigne à l'index 38640:")
    print(df.iloc[38640])

if __name__ == "__main__":
    df = load_and_filter_data()