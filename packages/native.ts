import path from 'path'

// Support Electron + Webpack + normal Node
const req: NodeRequire =
  typeof (global as any).__non_webpack_require__ === 'function'
    ? (global as any).__non_webpack_require__
    : require

const native = req('node-gyp-build')(path.join(__dirname, '..'))

export default native