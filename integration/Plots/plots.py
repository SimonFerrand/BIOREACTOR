import pandas as pd

def load_and_filter_data():
    # Chargement du fichier
    file_path = r"R:\Bioreactor\ServerFastAPI\data\data.csv"
    df = pd.read_csv(file_path, low_memory=False)
    print("\nShape initial du DataFrame:", df.shape)
    

if __name__ == "__main__":
    df = load_and_filter_data()