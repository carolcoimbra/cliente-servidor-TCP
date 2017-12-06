# Read dataTsne and plot each music (coord_X, coord_Y)
data <- read.csv("3MB/3M_estatisticas", head=F, sep=",", quote="")

medias <- c()
des <- c()
x <- seq(1,16)
for(i in seq(0,15)){
  x[i+1] <- data[1][[1]][i*10 + 1]
  medias <- c(medias, mean(data[3][[1]][seq(i*10 + 1, (i+1)*10)]))
  des <- c(des, sd(data[3][[1]][seq(i*10 + 1, (i+1)*10)]))
}

plot(log(x),medias,pch=16, cex=0.8)

for (i in seq(1, 16)){
  arrows(log(x[i]), medias[i]-des[i]*1.96/sqrt(10), log(x[i]), medias[i]+des[i]*1.96/sqrt(10), angle=90, length=0.05, code=3)
}