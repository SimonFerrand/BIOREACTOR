// vue.config.js
const webpack = require('webpack');

module.exports = {
  devServer: {
    port: 8080,
    allowedHosts: 'all',
  },
  configureWebpack: {
    plugins: [
      new webpack.DefinePlugin({
        '__VUE_OPTIONS_API__': JSON.stringify(true),
        '__VUE_PROD_DEVTOOLS__': JSON.stringify(false),
        '__VUE_PROD_HYDRATION_MISMATCH_DETAILS__': JSON.stringify(false)
      })
    ]
  }
};
