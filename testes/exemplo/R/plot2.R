# Read dataTsne and plot each music (coord_X, coord_Y)
data1 <- read.csv("3MB/e2", head=F, sep=",", quote="")
data2 <- read.csv("30MB/e2", head=F, sep=",", quote="")
data3 <- read.csv("300MB/e2", head=F, sep=",", quote="")

x <- c(3, 30, 300)

medias <- c(mean(data1[3][[1]]), mean(data2[3][[1]]), mean(data3[3][[1]]))
des <- c(sd(data1[3][[1]]), sd(data2[3][[1]]), sd(data3[3][[1]]))

plot(log(x),medias,pch=16,cex=0.8,type="bl")