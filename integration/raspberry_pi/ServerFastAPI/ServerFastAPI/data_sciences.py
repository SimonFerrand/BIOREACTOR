import pandas as pd

# Define the file path
'''
file_path = r'\\192.168.1.25\Raspberry\Bioreactor\ServerFastAPI\data\data.csv
# Read the CSV file into a DataFrame
df = pd.read_csv(file_path)
'''

# Define the file path
file_path = r'\\192.168.1.25\Raspberry\Bioreactor\ServerFastAPI\data\data.parquet'
# Lire le fichier Parquet
df = pd.read_parquet(file_path)

# Display the DataFrame
print(df.tail(30))
#print(df.info())
