# Lese die Textdatei "data.txt" ein
data <- read.table("data.txt", sep = ",", header = TRUE)

# Anzeigen der Tabelle
print(data)

data[,"t"] <- data[,"t"]/60000
plot(data[,"t"], data[,"T"], xlab="t [min]", ylab="T [C]")
pdf("firstplot.pdf")
plot(data[,"t"], data[,"T"], xlab="t [min]", ylab="T [C]")
dev.off()