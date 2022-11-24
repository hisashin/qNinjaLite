var path = require('path')
module.exports = {
  publicPath: '',
  configureWebpack: {
    output: {
      filename: "[name].js",
      chunkFilename: "[name]-chunk.js"
    },
    
    resolve: {
      alias: {
        'qpcr': path.resolve(__dirname, '../../qpcr/')
      },
    },
    devServer: {
      port: 8888,
      watchOptions: {
        poll: true
      },
      disableHostCheck: true
    }
  },
  css: {
    extract: {
      filename: '[name].css',
      chunkFilename: '[name].css',
    },
  }
};