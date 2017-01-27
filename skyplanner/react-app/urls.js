import AuthManager from './auth-manager'

class URLs {
    static get root() { return  URLs.__build_url('/')}
    static get login() { return URLs.__build_url('/login', 'login') }
    static get logout() { return URLs.__build_url('/logout', 'logout') }
    static get register() { return URLs.__build_url('/register', 'register') }
    static get equipment() { return URLs.__build_url('/equipment', 'equipment') }

    static buildPath(path, parameters) {
        if(Array.isArray(path))
            path = path.map( encodeURIComponent ).join('/');
        return [
            path,
            Object.keys(parameters).map( (key) => [encodeURIComponent(key), encodeURIComponent(parameters[key])].join('=') ).join('&')
        ].join('?');
    }

    static buildAuthPath(path, parameters) {
        return URLs.buildPath(path, Object.assign({auth: AuthManager.token()}, parameters) );
    }

    static __build_url(path, route) { return {path: path, route: route ? route : path}; }
};

export default URLs;
