import requests
from datetime import datetime

# 🌍 Configurações
OPENWEATHER_API_KEY = "700f052ece4fa77abad6f994f38c0255"
CITY = "São Paulo,BR"
SUPABASE_URL = "https://semsjwhpsiybhodlpxic.supabase.co/rest/v1/previsoes_tempo"
SUPABASE_KEY = "sb_secret_mmI71G0yKn4_GTr30HeRqw_jS1EcD0e"  # ⚠️ usar SECRET

def get_weather():
    """Consulta dados atuais e previsão (3h) no OpenWeather."""
    base_url = "https://api.openweathermap.org/data/2.5"
    
    # Clima atual
    current_url = f"{base_url}/weather?q={CITY}&appid={OPENWEATHER_API_KEY}&units=metric&lang=pt_br"
    forecast_url = f"{base_url}/forecast?q={CITY}&appid={OPENWEATHER_API_KEY}&units=metric&lang=pt_br"

    current = requests.get(current_url).json()
    forecast = requests.get(forecast_url).json()

    tempo_atual = {
        "temperatura": current["main"]["temp"],
        "umidade_ar": current["main"]["humidity"],
        "pressao": current["main"]["pressure"],
        "vento": current["wind"]["speed"]
    }

    # Pega só a próxima previsão (3h)
    prox = forecast["list"][0]
    chuva = prox.get("rain", {}).get("3h", 0.0)

    previsao = {
        "vai_chover": chuva > 0,
        "intensidade": chuva,
        "descricao": prox["weather"][0]["description"],
        "temperatura": prox["main"]["temp"],
        "umidade_ar": prox["main"]["humidity"],
        "pressao": prox["main"]["pressure"],
        "vento": prox["wind"]["speed"]
    }

    return tempo_atual, previsao

def salvar_previsao(previsao):
    """Envia dados para Supabase (tabela previsoes_tempo)."""
    headers = {
        "apikey": SUPABASE_KEY,
        "Authorization": f"Bearer {SUPABASE_KEY}",
        "Content-Type": "application/json"
    }
    r = requests.post(SUPABASE_URL, headers=headers, json=previsao)
    print("📡 Supabase resposta:", r.status_code, r.text)

if __name__ == "__main__":
    tempo_atual, previsao = get_weather()
    print("🌡️ Atual:", tempo_atual)
    print("☁️ Previsão próxima:", previsao)

    salvar_previsao(previsao)
