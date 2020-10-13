const fs = require('fs')
const Koa = require('koa')
const app = new Koa()
const zlib = require('zlib')
const crypto = require('crypto')
const data = zlib.gzipSync(crypto.randomBytes(1000))

app
    .use(async ctx => {
        ctx.set('Content-Encoding', 'gzip')
        ctx.body = data
    })