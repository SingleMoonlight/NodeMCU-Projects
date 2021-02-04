module.exports = {
    publicPath: "./",
    // 由于打包后的文件是存放在NodeMCU极小的闪存空间中的，
    // .map文件太大了放不下，因此要去掉打包的.map文件
    productionSourceMap: false,
};