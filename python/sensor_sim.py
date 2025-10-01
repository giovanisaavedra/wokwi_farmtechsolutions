import requests
import random
import time

SUPABASE_URL = "https://semsjwhpsiybhodlpxic.supabase.co/rest/v1/leituras_sensores"
SUPABASE_KEY = "sb_secret_mmI71G0yKn4_GTr30HeRqw_jS1EcD0e"  # usar SECRET

headers = {
    "apikey": SUPABASE_KEY,
    "Authorization": f"Bearer {SUPABASE_KEY}",
    "Content-Type": "application/json"
}

while True:
    # Simular botões NPK
    nitrogenio = random.choice([True, False])
    fosforo = random.choice([True, False])
    potassio = random.choice([True, False])

    # Simular pH (5.5 a 7.5)
    ph = round(random.uniform(5.5, 7.5), 2)

    # Simular umidade (30% a 90%)
    umidade = round(random.uniform(30, 90), 1)

    # Regras do tomate 🍅
    bomba = nitrogenio and fosforo and potassio and (6.0 <= ph <= 6.8) and umidade < 60

    data = {
        "nitrogenio": nitrogenio,
        "fosforo": fosforo,
        "potassio": potassio,
        "ph": ph,
        "umidade": umidade,
        "bomba": bomba
    }

    r = requests.post(SUPABASE_URL, headers=headers, json=data)
    print("📡 Enviado:", data, "| Resposta:", r.status_code)

    time.sleep(10)  # a cada 10 segundos
