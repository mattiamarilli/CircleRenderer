import pandas as pd
import matplotlib.pyplot as plt

# Leggi il CSV generato dal programma C++
csv_file = '../performancelog/performance_log.csv'  # Nome del file CSV generato dal codice C++

# Carica i dati
df = pd.read_csv(csv_file)

# Imposta il grafico
plt.figure(figsize=(10, 6))

# Disegna il grafico: Speedup vs Numero di Thread
for num_circles in df['Circles'].unique():
    subset = df[df['Circles'] == num_circles]
    plt.plot(subset['Threads'], subset['Speedup'], label=f'Circles: {num_circles}', marker='o')

# Aggiungi etichette e titolo
plt.title('Speedup vs Number of Threads for Different Circle Counts')
plt.xlabel('Number of Threads')
plt.ylabel('Speedup')
plt.legend(title="Circle Count", loc="best")

# Salva il grafico come immagine
plt.savefig('speedup_vs_threads.png')

# Mostra il grafico
plt.show()
