# FarmTech Solutions - Instalação de Dependências R
# Execute este script antes de rodar analise_farmtech.R

cat("📦 FarmTech Solutions - Instalação de Dependências R\n")
cat("==================================================\n")

# Lista de pacotes necessários
pacotes_necessarios <- c(
  "ggplot2",      # Gráficos avançados
  "dplyr",        # Manipulação de dados
  "corrplot",     # Matriz de correlação
  "forecast",     # Séries temporais
  "lubridate",    # Manipulação de datas
  "gridExtra",    # Múltiplos gráficos
  "scales"        # Formatação de escalas
)

# Função para instalar e verificar pacotes
instalar_dependencias <- function() {
  cat("🔍 Verificando pacotes instalados...\n")

  # Verificar quais pacotes estão faltando
  pacotes_instalados <- installed.packages()[,"Package"]
  pacotes_faltando <- pacotes_necessarios[!pacotes_necessarios %in% pacotes_instalados]

  if(length(pacotes_faltando) == 0) {
    cat("✅ Todos os pacotes já estão instalados!\n")
  } else {
    cat("📦 Instalando pacotes faltando:", paste(pacotes_faltando, collapse = ", "), "\n")
    cat("⏳ Isso pode demorar alguns minutos...\n")

    # Instalar pacotes faltando
    install.packages(pacotes_faltando, dependencies = TRUE, repos = "https://cran.r-project.org")

    cat("✅ Instalação concluída!\n")
  }

  # Verificar se a instalação foi bem-sucedida
  cat("\n🧪 Testando carregamento dos pacotes...\n")

  for(pacote in pacotes_necessarios) {
    tryCatch({
      library(pacote, character.only = TRUE, quietly = TRUE)
      cat("✅", pacote, "- OK\n")
    }, error = function(e) {
      cat("❌", pacote, "- ERRO:", e$message, "\n")
    })
  }

  cat("\n🎯 Dependências verificadas!\n")
  cat("💡 Agora você pode executar: source('analise_farmtech.R')\n")
}

# Função para verificar versão do R
verificar_versao_r <- function() {
  versao_r <- R.version.string
  cat("🔧 Versão do R:", versao_r, "\n")

  # Corrigir parsing da versão
  versao_major <- as.numeric(R.version$major)
  versao_minor <- as.numeric(R.version$minor)

  if(is.na(versao_major) || is.na(versao_minor)) {
    cat("✅ Versão do R detectada\n")
  } else {
    versao_numerica <- versao_major + versao_minor/10

    if(versao_numerica < 4.0) {
      cat("⚠️  Recomendamos R versão 4.0 ou superior\n")
      cat("📥 Download: https://cran.r-project.org/\n")
    } else {
      cat("✅ Versão do R adequada (", versao_major, ".", versao_minor, ")\n")
    }
  }
}

# Função para exibir informações do sistema
info_sistema <- function() {
  cat("\n💻 INFORMAÇÕES DO SISTEMA:\n")
  verificar_versao_r()
  cat("🖥️  Sistema:", Sys.info()["sysname"], "\n")
  cat("🏗️  Arquitetura:", Sys.info()["machine"], "\n")
  cat("📂 Diretório de trabalho:", getwd(), "\n")
}

# Executar instalação
main <- function() {
  info_sistema()
  cat("\n📦 INSTALAÇÃO DE DEPENDÊNCIAS:\n")
  instalar_dependencias()

  cat("\n📋 PRÓXIMOS PASSOS:\n")
  cat("1. Execute: source('analise_farmtech.R')\n")
  cat("2. Ou execute: main() para rodar a análise\n")
  cat("3. Os gráficos aparecerão automaticamente\n")
  cat("4. Um arquivo CSV será gerado com os dados\n")

  cat("\n🆘 EM CASO DE PROBLEMAS:\n")
  cat("- Verifique conexão com internet\n")
  cat("- Execute como administrador se necessário\n")
  cat("- Reinicie o R após instalação\n")

  cat("\n✅ Setup concluído! Projeto pronto para execução.\n")
}

# Executar automaticamente se script for rodado diretamente
if(!interactive()) {
  main()
} else {
  cat("💡 Execute main() para instalar as dependências\n")
  cat("💡 Ou execute install_dependencies() apenas para instalar\n")
}