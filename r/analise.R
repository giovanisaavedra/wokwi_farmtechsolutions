library(DBI)
library(RPostgres)
library(dplyr)
library(ggplot2)

# Conexão Supabase
con <- dbConnect(
  RPostgres::Postgres(),
  dbname = "postgres",
  host = "db.semsjwhpsiybhodlpxic.supabase.co",
  port = 5432,
  user = "postgres",
  password = "Farmtech@2025"
)

# Pega últimas leituras
leituras <- dbGetQuery(con, "
  SELECT created_at, umidade
  FROM leituras_sensores
  ORDER BY created_at DESC
  LIMIT 50;
")

# Converter datas para numérico (horas)
leituras <- leituras %>%
  arrange(created_at) %>%
  mutate(t = as.numeric(as.POSIXct(created_at)))

# Ajustar regressão linear simples: umidade ~ tempo
modelo <- lm(umidade ~ t, data = leituras)

# Previsões para as próximas 6 horas
t_futuro <- seq(max(leituras$t), max(leituras$t) + 6*3600, by = 600)
prev <- data.frame(
  t = t_futuro,
  created_at = as.POSIXct(t_futuro, origin = "1970-01-01"),
  umidade = predict(modelo, newdata = data.frame(t = t_futuro))
)

# Plotar
ggplot() +
  geom_point(data = leituras, aes(x = created_at, y = umidade), color = "blue") +
  geom_line(data = leituras, aes(x = created_at, y = umidade), color = "blue") +
  geom_line(data = prev, aes(x = created_at, y = umidade), color = "red", linetype = "dashed") +
  geom_hline(yintercept = 60, linetype = "dashed", color = "red") +
  geom_hline(yintercept = 80, linetype = "dashed", color = "green") +
  labs(
    title = "Umidade do Solo - Regressão Linear e Tendência",
    y = "Umidade (%)",
    x = "Data/Hora"
  ) +
  theme_minimal()
