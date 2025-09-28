# FarmTech Solutions - Análise Estatística de Irrigação
# Projeto: Otimização do sistema de irrigação inteligente para tomates
# Autor: Equipe FarmTech
# Data: Janeiro 2025

# ===== CONFIGURAÇÃO INICIAL =====
# Carregar bibliotecas necessárias
suppressMessages({
  library(ggplot2)     # Gráficos
  library(dplyr)       # Manipulação de dados
  library(corrplot)    # Matriz de correlação
  library(forecast)    # Séries temporais
  library(lubridate)   # Manipulação de datas
  library(gridExtra)   # Múltiplos gráficos
  library(scales)      # Formatação de escalas
})

# ===== FUNÇÃO PARA GERAR DADOS SIMULADOS =====
gerar_dados_irrigacao <- function(n_dias = 30) {
  cat("📊 Gerando dados simulados de irrigação para", n_dias, "dias...\n")

  set.seed(42)  # Para reproduzibilidade

  # Gerar dados base
  dados <- data.frame(
    dia = 1:n_dias,
    data = seq(from = as.Date("2025-01-01"),
               by = "day",
               length.out = n_dias)
  )

  # Simular variáveis ambientais realistas para tomate
  dados$temperatura <- pmax(15, pmin(35, rnorm(n_dias, mean = 24, sd = 4)))
  dados$umidade_solo <- pmax(20, pmin(95, rnorm(n_dias, mean = 65, sd = 15)))
  dados$ph_solo <- pmax(5.0, pmin(8.0, rnorm(n_dias, mean = 6.4, sd = 0.5)))
  dados$umidade_ar <- pmax(30, pmin(90, rnorm(n_dias, mean = 70, sd = 12)))

  # Simular presença de nutrientes NPK (0 = inadequado, 1 = adequado)
  dados$nitrogenio <- rbinom(n_dias, 1, 0.75)
  dados$fosforo <- rbinom(n_dias, 1, 0.80)
  dados$potassio <- rbinom(n_dias, 1, 0.85)
  dados$npk_completo <- dados$nitrogenio * dados$fosforo * dados$potassio

  # Simular previsão meteorológica (0 = sem chuva, 1 = chuva prevista)
  dados$previsao_chuva <- rbinom(n_dias, 1, 0.25)
  dados$intensidade_chuva <- ifelse(dados$previsao_chuva == 1,
                                   runif(n_dias, 2, 15), 0)

  # Calcular tempo de irrigação baseado na lógica do ESP32
  dados$irrigacao_minutos <- with(dados, {
    # Condições ideais para tomate
    ph_adequado <- (ph_solo >= 6.0 & ph_solo <= 6.8)
    umidade_adequada <- (umidade_solo >= 60 & umidade_solo <= 80)
    npk_ok <- (npk_completo == 1)
    sem_chuva <- (previsao_chuva == 0 | intensidade_chuva < 2.0)

    # Tempo base de irrigação
    tempo_base <- 45

    # Ajustes baseados nas condições
    ajuste_temp <- (temperatura - 24) * 1.5
    ajuste_umidade <- (65 - umidade_solo) * 0.8
    ajuste_ph <- ifelse(ph_adequado, 0, 15)
    ajuste_npk <- ifelse(npk_ok, 0, 20)
    ajuste_chuva <- ifelse(sem_chuva, 0, -30)

    # Calcular tempo final de irrigação
    tempo_irrigacao <- tempo_base +
                      ajuste_temp +
                      ajuste_umidade +
                      ajuste_ph +
                      ajuste_npk +
                      ajuste_chuva +
                      rnorm(n_dias, 0, 3)

    # Garantir que não seja negativo e limitado a 120 minutos
    pmax(0, pmin(120, tempo_irrigacao))
  })

  # Calcular eficiência do sistema (%)
  dados$eficiencia <- with(dados, {
    base_eficiencia <- 80

    # Bônus por condições ideais
    bonus_ph <- ifelse(ph_solo >= 6.0 & ph_solo <= 6.8, 10, 0)
    bonus_umidade <- ifelse(umidade_solo >= 60 & umidade_solo <= 80, 8, 0)
    bonus_npk <- ifelse(npk_completo == 1, 12, 0)
    bonus_clima <- ifelse(temperatura >= 20 & temperatura <= 26, 5, 0)

    eficiencia_total <- base_eficiencia +
                       bonus_ph +
                       bonus_umidade +
                       bonus_npk +
                       bonus_clima +
                       rnorm(n_dias, 0, 2)

    pmax(50, pmin(100, eficiencia_total))
  })

  # Calcular economia de água (litros)
  dados$economia_agua <- with(dados, {
    irrigacao_tradicional <- 60  # minutos de irrigação tradicional
    economia_minutos <- pmax(0, irrigacao_tradicional - irrigacao_minutos)
    economia_minutos * 2.5  # 2.5 litros por minuto economizados
  })

  return(dados)
}

# ===== ANÁLISE EXPLORATÓRIA =====
analise_exploratoria <- function(dados) {
  cat("\n📈 === ANÁLISE EXPLORATÓRIA ===\n")

  # Estatísticas descritivas
  cat("\n📊 Estatísticas Descritivas:\n")
  variaveis_numericas <- c("temperatura", "umidade_solo", "ph_solo",
                          "irrigacao_minutos", "eficiencia", "economia_agua")

  # Corrigir a função summary para exibir corretamente
  sumario <- summary(dados[, variaveis_numericas])
  print(sumario)

  # Correlações
  cat("\n🔗 Análise de Correlação:\n")
  cor_matrix <- cor(dados[, variaveis_numericas], use = "complete.obs")
  print(round(cor_matrix, 3))

  # Visualizar matriz de correlação
  corrplot(cor_matrix, method = "color", type = "upper",
           order = "hclust", tl.cex = 0.8, tl.col = "black",
           title = "Matriz de Correlação - Variáveis de Irrigação",
           mar = c(0,0,2,0))

  # Análise de frequências
  cat("\n📋 Frequências de Nutrientes:\n")
  cat("Nitrogênio adequado:", sum(dados$nitrogenio), "/", nrow(dados),
      sprintf("(%.1f%%)\n", mean(dados$nitrogenio) * 100))
  cat("Fósforo adequado:", sum(dados$fosforo), "/", nrow(dados),
      sprintf("(%.1f%%)\n", mean(dados$fosforo) * 100))
  cat("Potássio adequado:", sum(dados$potassio), "/", nrow(dados),
      sprintf("(%.1f%%)\n", mean(dados$potassio) * 100))
  cat("NPK completo:", sum(dados$npk_completo), "/", nrow(dados),
      sprintf("(%.1f%%)\n", mean(dados$npk_completo) * 100))

  return(cor_matrix)
}

# ===== VISUALIZAÇÕES =====
criar_graficos_principais <- function(dados) {
  cat("\n📊 Gerando gráficos principais...\n")

  # Gráfico 1: Tendência de irrigação ao longo do tempo
  p1 <- ggplot(dados, aes(x = data, y = irrigacao_minutos)) +
    geom_line(color = "blue", size = 1, alpha = 0.8) +
    geom_smooth(method = "loess", color = "red", se = TRUE, alpha = 0.3) +
    geom_hline(yintercept = 45, linetype = "dashed", color = "green",
               alpha = 0.7) +
    labs(title = "📈 Irrigação Diária ao Longo do Tempo",
         subtitle = "Linha verde = irrigação ideal (45 min)",
         x = "Data",
         y = "Irrigação (minutos)") +
    theme_minimal() +
    theme(plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
          plot.subtitle = element_text(hjust = 0.5, size = 11))

  # Gráfico 2: Relação pH x Eficiência
  p2 <- ggplot(dados, aes(x = ph_solo, y = eficiencia)) +
    geom_point(aes(color = irrigacao_minutos, size = economia_agua), alpha = 0.7) +
    geom_smooth(method = "lm", color = "darkgreen", se = TRUE) +
    geom_vline(xintercept = c(6.0, 6.8), linetype = "dashed",
               color = "red", alpha = 0.7) +
    scale_color_gradient(low = "lightblue", high = "darkblue",
                        name = "Irrigação\n(min)") +
    scale_size_continuous(name = "Economia\n(litros)") +
    labs(title = "🧪 pH do Solo vs. Eficiência do Sistema",
         subtitle = "Linhas vermelhas = faixa ideal para tomate (6.0-6.8)",
         x = "pH do Solo",
         y = "Eficiência (%)") +
    theme_minimal() +
    theme(plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
          plot.subtitle = element_text(hjust = 0.5, size = 11))

  # Gráfico 3: Comparação de eficiência por NPK
  dados$npk_status <- factor(dados$npk_completo,
                            levels = c(0, 1),
                            labels = c("NPK Inadequado", "NPK Completo"))

  p3 <- ggplot(dados, aes(x = npk_status, y = eficiencia, fill = npk_status)) +
    geom_boxplot(alpha = 0.7, outlier.alpha = 0.5) +
    geom_jitter(width = 0.2, alpha = 0.5, size = 2) +
    stat_summary(fun = mean, geom = "point", shape = 23, size = 4,
                fill = "yellow", color = "black") +
    scale_fill_manual(values = c("NPK Inadequado" = "#ff7f7f",
                                "NPK Completo" = "#7fff7f")) +
    labs(title = "🌱 Eficiência por Status do NPK",
         subtitle = "Losango amarelo = média",
         x = "Status dos Nutrientes",
         y = "Eficiência (%)",
         fill = "NPK Status") +
    theme_minimal() +
    theme(plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
          plot.subtitle = element_text(hjust = 0.5, size = 11),
          legend.position = "none")

  # Gráfico 4: Economia de água
  p4 <- ggplot(dados, aes(x = irrigacao_minutos, y = economia_agua)) +
    geom_point(aes(color = eficiencia), size = 3, alpha = 0.7) +
    geom_smooth(method = "lm", color = "blue", se = TRUE) +
    scale_color_gradient(low = "red", high = "green", name = "Eficiência\n(%)") +
    labs(title = "💧 Irrigação vs. Economia de Água",
         subtitle = "Relação entre tempo de irrigação e economia hídrica",
         x = "Irrigação (minutos)",
         y = "Economia de Água (litros)") +
    theme_minimal() +
    theme(plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
          plot.subtitle = element_text(hjust = 0.5, size = 11))

  # Exibir gráficos
  print(p1)
  print(p2)
  print(p3)
  print(p4)

  return(list(p1 = p1, p2 = p2, p3 = p3, p4 = p4))
}

# ===== MODELO PREDITIVO =====
criar_modelo_preditivo <- function(dados) {
  cat("\n🤖 === MODELO PREDITIVO ===\n")

  # Modelo 1: Predição do tempo de irrigação
  modelo_irrigacao <- lm(irrigacao_minutos ~ temperatura + umidade_solo +
                        ph_solo + npk_completo + previsao_chuva +
                        intensidade_chuva, data = dados)

  cat("\n📊 Modelo 1 - Predição de Irrigação:\n")
  print(summary(modelo_irrigacao))

  # Modelo 2: Predição da eficiência
  modelo_eficiencia <- lm(eficiencia ~ temperatura + umidade_solo +
                         ph_solo + npk_completo + irrigacao_minutos,
                         data = dados)

  cat("\n📊 Modelo 2 - Predição de Eficiência:\n")
  print(summary(modelo_eficiencia))

  # Adicionar predições aos dados
  dados$irrigacao_predita <- predict(modelo_irrigacao, dados)
  dados$eficiencia_predita <- predict(modelo_eficiencia, dados)

  # Calcular métricas de qualidade
  rmse_irrigacao <- sqrt(mean((dados$irrigacao_minutos - dados$irrigacao_predita)^2))
  rmse_eficiencia <- sqrt(mean((dados$eficiencia - dados$eficiencia_predita)^2))

  cat(sprintf("\n✅ RMSE Irrigação: %.2f minutos\n", rmse_irrigacao))
  cat(sprintf("✅ RMSE Eficiência: %.2f%%\n", rmse_eficiencia))

  # Gráfico de predições vs. real
  p_pred <- ggplot(dados, aes(x = irrigacao_minutos, y = irrigacao_predita)) +
    geom_point(alpha = 0.7, color = "darkblue", size = 2) +
    geom_abline(intercept = 0, slope = 1, color = "red", linetype = "dashed", size = 1) +
    geom_smooth(method = "lm", color = "green", se = TRUE, alpha = 0.3) +
    labs(title = "🎯 Predições vs. Valores Reais - Irrigação",
         subtitle = "Linha vermelha = predição perfeita",
         x = "Irrigação Real (minutos)",
         y = "Irrigação Predita (minutos)") +
    theme_minimal() +
    theme(plot.title = element_text(hjust = 0.5, size = 14, face = "bold"),
          plot.subtitle = element_text(hjust = 0.5, size = 11))

  print(p_pred)

  return(list(modelo_irrigacao = modelo_irrigacao,
              modelo_eficiencia = modelo_eficiencia,
              rmse_irrigacao = rmse_irrigacao,
              rmse_eficiencia = rmse_eficiencia))
}

# ===== RECOMENDAÇÕES OTIMIZADAS =====
gerar_recomendacoes <- function(dados, modelos) {
  cat("\n💡 === RECOMENDAÇÕES PARA OTIMIZAÇÃO ===\n")

  # Cenário ótimo para tomate
  cenario_otimo <- data.frame(
    temperatura = 23,
    umidade_solo = 70,
    ph_solo = 6.4,
    npk_completo = 1,
    previsao_chuva = 0,
    intensidade_chuva = 0
  )

  irrigacao_otima <- predict(modelos$modelo_irrigacao, cenario_otimo)
  eficiencia_otima <- predict(modelos$modelo_eficiencia,
                             cbind(cenario_otimo, irrigacao_minutos = irrigacao_otima))

  cat(sprintf("🎯 Irrigação ótima predita: %.1f minutos/dia\n", irrigacao_otima))
  cat(sprintf("🎯 Eficiência ótima predita: %.1f%%\n", eficiencia_otima))

  # Análises estatísticas
  irrigacao_media <- mean(dados$irrigacao_minutos)
  eficiencia_media <- mean(dados$eficiencia)
  economia_total <- sum(dados$economia_agua)

  cat(sprintf("\n📊 Situação Atual:\n"))
  cat(sprintf("   ⏱️  Irrigação média: %.1f minutos/dia\n", irrigacao_media))
  cat(sprintf("   📈 Eficiência média: %.1f%%\n", eficiencia_media))
  cat(sprintf("   💧 Economia total: %.1f litros/mês\n", economia_total))

  # Potencial de melhoria
  economia_potencial <- max(0, irrigacao_media - irrigacao_otima)
  melhoria_eficiencia <- max(0, eficiencia_otima - eficiencia_media)

  cat(sprintf("\n🚀 Potencial de Otimização:\n"))
  cat(sprintf("   ⏰ Redução de irrigação: %.1f min/dia (%.1f%%)\n",
              economia_potencial, (economia_potencial/irrigacao_media)*100))
  cat(sprintf("   📈 Melhoria de eficiência: %.1f pontos percentuais\n",
              melhoria_eficiencia))

  # Análise de condições críticas
  condicoes_criticas <- dados %>%
    filter(eficiencia < 85 | irrigacao_minutos > 70) %>%
    nrow()

  cat(sprintf("   ⚠️  Dias sub-ótimos: %d/%d (%.1f%%)\n",
              condicoes_criticas, nrow(dados),
              (condicoes_criticas/nrow(dados))*100))

  # Recomendações específicas
  cat("\n📋 RECOMENDAÇÕES ESPECÍFICAS:\n")
  cat("1. 🎯 Manter pH entre 6.0-6.8 maximiza eficiência em 10-15%\n")
  cat("2. 💧 Umidade do solo ideal: 60-80% (zona de conforto do tomate)\n")
  cat("3. 🌱 NPK completo reduz necessidade de irrigação em ~20 min/dia\n")
  cat("4. 🌡️  Temperatura ideal: 20-25°C para máxima eficiência\n")
  cat("5. 🌧️  Monitoramento meteorológico evita irrigação desnecessária\n")
  cat("6. ⏰ Irrigação > 70 min/dia indica problemas no sistema\n")
  cat("7. 📊 Eficiência < 85% requer revisão dos parâmetros\n")

  # ROI estimado
  custo_agua_tradicional <- irrigacao_media * 30 * 0.005  # R$ 0.005 por minuto
  custo_agua_otimizado <- irrigacao_otima * 30 * 0.005
  economia_mensal <- custo_agua_tradicional - custo_agua_otimizado

  cat(sprintf("\n💰 ANÁLISE ECONÔMICA (estimativa):\n"))
  cat(sprintf("   💵 Custo mensal atual: R$ %.2f\n", custo_agua_tradicional))
  cat(sprintf("   💵 Custo mensal otimizado: R$ %.2f\n", custo_agua_otimizado))
  cat(sprintf("   💰 Economia mensal: R$ %.2f (%.1f%%)\n",
              economia_mensal, (economia_mensal/custo_agua_tradicional)*100))
  cat(sprintf("   💰 Economia anual: R$ %.2f\n", economia_mensal * 12))
}

# ===== RELATÓRIO EXECUTIVO =====
gerar_relatorio_executivo <- function(dados, modelos) {
  cat("\n📄 === RELATÓRIO EXECUTIVO ===\n")
  cat("🌱 FarmTech Solutions - Análise de Otimização de Irrigação\n")
  cat("🍅 Cultura: Tomate\n")
  cat("📅 Período analisado:", format(min(dados$data), "%d/%m/%Y"),
      "a", format(max(dados$data), "%d/%m/%Y"), "\n")
  cat("📊 Amostra:", nrow(dados), "dias de dados\n")

  # KPIs principais
  cat("\n🎯 INDICADORES PRINCIPAIS:\n")
  cat(sprintf("   ⏱️  Irrigação média: %.1f min/dia\n", mean(dados$irrigacao_minutos)))
  cat(sprintf("   📈 Eficiência média: %.1f%%\n", mean(dados$eficiencia)))
  cat(sprintf("   💧 Economia de água: %.1f L/mês\n", sum(dados$economia_agua)))
  cat(sprintf("   🌱 NPK adequado: %.1f%% dos dias\n", mean(dados$npk_completo) * 100))
  cat(sprintf("   🧪 pH ideal: %.1f%% dos dias\n",
              mean(dados$ph_solo >= 6.0 & dados$ph_solo <= 6.8) * 100))

  # Qualidade dos modelos
  cat("\n🤖 QUALIDADE DOS MODELOS:\n")
  cat(sprintf("   📊 R² Irrigação: %.3f\n", summary(modelos$modelo_irrigacao)$r.squared))
  cat(sprintf("   📊 R² Eficiência: %.3f\n", summary(modelos$modelo_eficiencia)$r.squared))
  cat(sprintf("   📏 RMSE Irrigação: %.2f min\n", modelos$rmse_irrigacao))
  cat(sprintf("   📏 RMSE Eficiência: %.2f%%\n", modelos$rmse_eficiencia))

  # Conclusões
  cat("\n✅ CONCLUSÕES:\n")
  cat("1. ✅ Sistema de irrigação inteligente reduz consumo de água\n")
  cat("2. ✅ Monitoramento de pH é crítico para eficiência\n")
  cat("3. ✅ NPK completo otimiza tempo de irrigação\n")
  cat("4. ✅ Previsão meteorológica evita desperdício\n")
  cat("5. ✅ Modelos preditivos auxiliam tomada de decisão\n")

  cat("\n📈 PRÓXIMOS PASSOS:\n")
  cat("1. 🔄 Implementar otimizações sugeridas\n")
  cat("2. 📊 Monitorar KPIs em tempo real\n")
  cat("3. 🤖 Refinar modelos com mais dados\n")
  cat("4. 🌱 Expandir para outras culturas\n")
  cat("5. 💰 Calcular ROI preciso\n")
}

# ===== FUNÇÃO PRINCIPAL =====
main <- function() {
  cat("🌱 FarmTech Solutions - Análise Estatística Avançada\n")
  cat("🍅 Sistema de Irrigação Inteligente para Tomates\n")
  cat(rep("=", 60), "\n", sep = "")

  # Verificar e instalar pacotes se necessário
  verificar_pacotes()

  # Gerar dados simulados
  cat("📊 Gerando dados simulados baseados no sistema ESP32...\n")
  dados <- gerar_dados_irrigacao(30)

  # Salvar dados para referência
  write.csv(dados, "dados_analise_farmtech.csv", row.names = FALSE)
  cat("💾 Dados salvos em 'dados_analise_farmtech.csv'\n")

  # Análise exploratória
  cor_matrix <- analise_exploratoria(dados)

  # Criar visualizações
  graficos <- criar_graficos_principais(dados)

  # Criar modelos preditivos
  modelos <- criar_modelo_preditivo(dados)

  # Gerar recomendações
  gerar_recomendacoes(dados, modelos)

  # Relatório executivo
  gerar_relatorio_executivo(dados, modelos)

  cat("\n✅ ANÁLISE CONCLUÍDA COM SUCESSO!\n")
  cat("📁 Arquivos gerados:\n")
  cat("   📊 dados_analise_farmtech.csv - Dataset completo\n")
  cat("   📈 Gráficos exibidos no console\n")
  cat("   📄 Relatório impresso no console\n")

  cat("\n🚀 Para usar em produção:\n")
  cat("1. Substitua dados simulados por dados reais do ESP32\n")
  cat("2. Ajuste parâmetros conforme cultura específica\n")
  cat("3. Implemente recomendações no sistema de irrigação\n")
  cat("4. Monitore KPIs continuamente\n")

  return(list(dados = dados, modelos = modelos, correlacoes = cor_matrix))
}

# ===== FUNÇÃO AUXILIAR PARA VERIFICAR PACOTES =====
verificar_pacotes <- function() {
  pacotes_necessarios <- c("ggplot2", "dplyr", "corrplot", "forecast",
                          "lubridate", "gridExtra", "scales")

  pacotes_faltando <- pacotes_necessarios[!pacotes_necessarios %in%
                                         installed.packages()[,"Package"]]

  if(length(pacotes_faltando) > 0) {
    cat("📦 Instalando pacotes necessários:", paste(pacotes_faltando, collapse = ", "), "\n")
    install.packages(pacotes_faltando, dependencies = TRUE)
  }
}

# ===== EXECUTAR ANÁLISE =====
if (interactive()) {
  cat("🚀 Iniciando análise estatística...\n")
  cat("💡 Pressione Ctrl+C para interromper\n\n")

  # Executar análise principal
  resultados <- main()

  cat("\n📊 Análise finalizada! Resultados disponíveis em 'resultados'\n")
  cat("💡 Digite 'View(resultados$dados)' para ver os dados\n")
  cat("💡 Digite 'summary(resultados$modelos$modelo_irrigacao)' para ver modelo\n")

} else {
  cat("📄 Script carregado. Execute main() para iniciar a análise.\n")
  cat("💡 Ou execute source('este_arquivo.R') no console do R\n")
}