"""
FarmTech Solutions - Integração API Meteorológica
Consulta previsão do tempo para otimizar irrigação no sistema ESP32
Versão 2.0 - Compatível com prog1.ino atualizado
"""

import requests
import json
import time
from datetime import datetime, timedelta
import sys

class WeatherAPI:
    def __init__(self, api_key, cidade="São Paulo"):
        self.api_key = api_key
        self.cidade = cidade
        self.base_url = "http://api.openweathermap.org/data/2.5"
        
    def consultar_previsao(self):
        """Consulta previsão do tempo nas próximas 6 horas"""
        try:
            url = f"{self.base_url}/forecast"
            params = {
                'q': self.cidade,
                'appid': self.api_key,
                'units': 'metric',
                'cnt': 2  # Próximas 6 horas (2 períodos de 3h)
            }
            
            response = requests.get(url, params=params)
            response.raise_for_status()
            data = response.json()
            
            # Verifica se há previsão de chuva
            vai_chover = False
            intensidade_chuva = 0
            descricao = "Limpo"
            
            for forecast in data['list']:
                if 'rain' in forecast:
                    vai_chover = True
                    # Soma a intensidade da chuva (mm/3h)
                    intensidade_chuva += forecast['rain'].get('3h', 0)
                    descricao = forecast['weather'][0]['description']
                elif 'snow' in forecast:
                    vai_chover = True
                    intensidade_chuva += forecast['snow'].get('3h', 0)
                    descricao = forecast['weather'][0]['description']
                else:
                    descricao = forecast['weather'][0]['description']
            
            return {
                'vai_chover': vai_chover,
                'intensidade': round(intensidade_chuva, 1),
                'descricao': descricao,
                'timestamp': datetime.now().isoformat()
            }
            
        except requests.exceptions.RequestException as e:
            print(f"❌ Erro de conexão: {e}")
            return self._retorno_erro()
        except KeyError as e:
            print(f"❌ Erro nos dados da API: {e}")
            return self._retorno_erro()
        except Exception as e:
            print(f"❌ Erro inesperado: {e}")
            return self._retorno_erro()
    
    def consultar_tempo_atual(self):
        """Consulta condições meteorológicas atuais"""
        try:
            url = f"{self.base_url}/weather"
            params = {
                'q': self.cidade,
                'appid': self.api_key,
                'units': 'metric'
            }
            
            response = requests.get(url, params=params)
            response.raise_for_status()
            data = response.json()
            
            return {
                'temperatura': round(data['main']['temp'], 1),
                'umidade_ar': data['main']['humidity'],
                'pressao': data['main']['pressure'],
                'descricao': data['weather'][0]['description'],
                'vento': round(data['wind']['speed'], 1),
                'nuvens': data['clouds']['all']
            }
            
        except Exception as e:
            print(f"❌ Erro ao consultar tempo atual: {e}")
            return None
    
    def _retorno_erro(self):
        """Retorna valores padrão em caso de erro"""
        return {
            'vai_chover': False,
            'intensidade': 0.0,
            'descricao': 'Erro na consulta',
            'timestamp': datetime.now().isoformat()
        }

class ESP32CodeGenerator:
    """Gera código C++ para ser copiado no ESP32"""
    
    def __init__(self):
        self.dados_gerados = []
    
    def gerar_codigo_cpp(self, previsao, tempo_atual=None):
        """Gera código C++ para ser copiado no prog1.ino"""
        
        # Determinar valores booleanos
        vai_chover = previsao['vai_chover']
        intensidade = previsao['intensidade']
        descricao = previsao['descricao']
        
        # Gerar código C++
        codigo_cpp = f"""
// ===== DADOS METEOROLÓGICOS ATUALIZADOS =====
// Gerado automaticamente em: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
// Cidade: Consultada via API OpenWeather

bool previsao_chuva = {"true" if vai_chover else "false"};
float intensidade_chuva = {intensidade}f;
String condicao_tempo = "{descricao}";
"""
        
        # Adicionar comentários explicativos
        comentarios = f"""
/*
 * INSTRUÇÕES PARA USO NO WOKWI:
 * 1. Copie as 3 linhas acima
 * 2. Substitua as variáveis correspondentes no prog1.ino
 * 3. Recompile o código no Wokwi
 * 4. Teste o comportamento do sistema
 * 
 * DADOS DA CONSULTA:
 * - Timestamp: {previsao['timestamp']}
 * - Vai chover: {"Sim" if vai_chover else "Não"}
 * - Intensidade: {intensidade} mm
 * - Condição: {descricao}
"""
        
        if tempo_atual:
            comentarios += f"""
 * 
 * CONDIÇÕES ATUAIS:
 * - Temperatura: {tempo_atual['temperatura']}°C
 * - Umidade ar: {tempo_atual['umidade_ar']}%
 * - Pressão: {tempo_atual['pressao']} hPa
 * - Vento: {tempo_atual['vento']} m/s
 */
"""
        else:
            comentarios += " */"
        
        return codigo_cpp + comentarios
    
    def salvar_dados_csv(self, previsao, tempo_atual, arquivo="dados_meteorologicos.csv"):
        """Salva dados em CSV para análise posterior"""
        import csv
        import os
        
        # Verificar se arquivo existe para adicionar cabeçalho
        arquivo_existe = os.path.exists(arquivo)
        
        with open(arquivo, 'a', newline='', encoding='utf-8') as csvfile:
            fieldnames = ['timestamp', 'vai_chover', 'intensidade_chuva', 'descricao',
                         'temperatura', 'umidade_ar', 'pressao', 'vento']
            
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            
            # Escrever cabeçalho se arquivo não existir
            if not arquivo_existe:
                writer.writeheader()
            
            # Preparar dados
            dados = {
                'timestamp': previsao['timestamp'],
                'vai_chover': previsao['vai_chover'],
                'intensidade_chuva': previsao['intensidade'],
                'descricao': previsao['descricao'],
                'temperatura': tempo_atual['temperatura'] if tempo_atual else 'N/A',
                'umidade_ar': tempo_atual['umidade_ar'] if tempo_atual else 'N/A',
                'pressao': tempo_atual['pressao'] if tempo_atual else 'N/A',
                'vento': tempo_atual['vento'] if tempo_atual else 'N/A'
            }
            
            writer.writerow(dados)
            self.dados_gerados.append(dados)

def exibir_banner():
    """Exibe banner inicial do programa"""
    print("="*60)
    print("🌱 FarmTech Solutions - Weather API Integration v2.0")
    print("🌤️  Integração com OpenWeatherMap para Sistema de Irrigação")
    print("📡 Compatível com prog1.ino v2.0")
    print("="*60)

def obter_api_key():
    """Obtém API key do arquivo .env ou solicita input"""
    import os
    from dotenv import load_dotenv
    
    # Carregar variáveis do arquivo .env
    load_dotenv()
    
    # Tentar obter da variável de ambiente (.env)
    api_key = os.getenv('OPENWEATHER_API_KEY')
    
    if api_key:
        print("🔑 API Key carregada do arquivo .env")
        return api_key
    
    # Se não encontrou no .env, solicitar input
    print("\n🔑 API Key não encontrada no arquivo .env!")
    print("📋 Para obter uma API key gratuita:")
    print("   1. Acesse: https://openweathermap.org/api")
    print("   2. Crie uma conta gratuita")
    print("   3. Copie sua API key")
    print("   4. Adicione no arquivo .env: OPENWEATHER_API_KEY=sua_key_aqui")
    print()
    
    api_key = input("🔑 Digite sua API key do OpenWeatherMap: ").strip()
    
    if not api_key:
        print("❌ API key é obrigatória!")
        return None
    
    return api_key

def main():
    """Função principal do programa"""
    
    exibir_banner()
    
    # Carregar configurações do .env
    from dotenv import load_dotenv
    import os
    
    load_dotenv()
    
    # Configurações (com fallback para valores padrão)
    CIDADE = os.getenv('CIDADE', 'São Paulo')
    MODO_DEMONSTRACAO = os.getenv('MODO_SIMULACAO', 'True').lower() != 'false'
    
    # Obter API key
    api_key = obter_api_key()
    if not api_key:
        print("❌ Programa encerrado - API key necessária")
        return
    
    # Inicializar componentes
    weather = WeatherAPI(api_key, CIDADE)
    code_gen = ESP32CodeGenerator()
    
    print(f"\n🏙️  Cidade configurada: {CIDADE}")
    print(f"🔧 Modo demonstração: {'Ativo' if MODO_DEMONSTRACAO else 'Loop contínuo'}")
    print("\n🔄 Iniciando consultas meteorológicas...")
    
    try:
        num_consultas = 3 if MODO_DEMONSTRACAO else 999999
        
        for i in range(num_consultas):
            print(f"\n{'='*50}")
            print(f"🔄 CONSULTA {i+1}" + (f"/{num_consultas}" if MODO_DEMONSTRACAO else ""))
            print(f"🕐 {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
            print("="*50)
            
            # Consultar APIs
            print("📡 Consultando previsão meteorológica...")
            previsao = weather.consultar_previsao()
            
            print("📡 Consultando condições atuais...")
            tempo_atual = weather.consultar_tempo_atual()
            
            # Exibir resultados
            print(f"\n📊 === DADOS METEOROLÓGICOS ===")
            
            if tempo_atual:
                print(f"🌡️  Temperatura atual: {tempo_atual['temperatura']}°C")
                print(f"💧 Umidade do ar: {tempo_atual['umidade_ar']}%")
                print(f"🌪️  Vento: {tempo_atual['vento']} m/s")
                print(f"☁️  Nuvens: {tempo_atual['nuvens']}%")
                print(f"🌤️  Condição atual: {tempo_atual['descricao']}")
            
            print(f"\n🌧️  Previsão (próximas 6h):")
            print(f"   Vai chover: {'Sim' if previsao['vai_chover'] else 'Não'}")
            print(f"   Intensidade: {previsao['intensidade']} mm")
            print(f"   Condição: {previsao['descricao']}")
            
            # Gerar código C++
            codigo_cpp = code_gen.gerar_codigo_cpp(previsao, tempo_atual)
            
            # Salvar dados em CSV
            code_gen.salvar_dados_csv(previsao, tempo_atual)
            
            # Exibir instruções
            print(f"\n{'='*60}")
            print("📋 CÓDIGO PARA COPIAR NO WOKWI (prog1.ino):")
            print("="*60)
            print(codigo_cpp)
            print("="*60)
            
            # Recomendações baseadas nos dados
            print(f"\n💡 RECOMENDAÇÕES PARA O SISTEMA:")
            if previsao['vai_chover']:
                if previsao['intensidade'] > 5.0:
                    print("🚨 Chuva intensa prevista - Suspender irrigação imediatamente")
                    print("📤 Comando sugerido: PARADA_FORCADA")
                else:
                    print("🌧️ Chuva leve prevista - Reduzir irrigação")
                    print("📤 Comando sugerido: SUSPENDER_CHUVA")
            else:
                print("☀️ Sem chuva prevista - Irrigação normal permitida")
                print("📤 Comando sugerido: PERMITIR_IRRIGACAO")
            
            # Aguardar próxima consulta
            if i < num_consultas - 1:
                intervalo = 10 if MODO_DEMONSTRACAO else 300  # 10s demo, 5min produção
                print(f"\n⏰ Aguardando {intervalo} segundos para próxima consulta...")
                
                if MODO_DEMONSTRACAO:
                    print("💡 Pressione Ctrl+C para interromper")
                
                time.sleep(intervalo)
        
        # Resumo final
        print(f"\n{'='*60}")
        print("📊 RESUMO DA SESSÃO:")
        print(f"✅ {len(code_gen.dados_gerados)} consultas realizadas com sucesso")
        print(f"💾 Dados salvos em: dados_meteorologicos.csv")
        print("🎯 Códigos C++ gerados e prontos para uso no Wokwi")
        print("="*60)
            
    except KeyboardInterrupt:
        print("\n⏹️  Interrompido pelo usuário")
    except Exception as e:
        print(f"\n❌ Erro inesperado: {e}")
    
    finally:
        print("\n🏁 FarmTech Weather Integration finalizado")
        print("💡 Para usar os dados:")
        print("   1. Copie o código C++ gerado")
        print("   2. Substitua as variáveis no prog1.ino")
        print("   3. Recompile no Wokwi")
        print("   4. Teste o comportamento do sistema")

if __name__ == "__main__":
    print("🐍 Iniciando FarmTech Weather Integration...")
    print("💡 Pressione Ctrl+C a qualquer momento para parar")
    print()
    
    # Verificar se bibliotecas estão instaladas
    try:
        import requests
        from dotenv import load_dotenv
    except ImportError as e:
        if 'requests' in str(e):
            print("❌ Biblioteca 'requests' não encontrada!")
            print("📦 Instale com: pip install requests")
        elif 'dotenv' in str(e):
            print("❌ Biblioteca 'python-dotenv' não encontrada!")
            print("📦 Instale com: pip install python-dotenv")
        else:
            print(f"❌ Biblioteca não encontrada: {e}")
            print("📦 Instale com: pip install requests python-dotenv")
        sys.exit(1)
    
    main()