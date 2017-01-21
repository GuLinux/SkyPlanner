let __instance = null;

class AuthManager {
    constructor() {
        this.observers = [];
        this.user = null;
    }

    static instance() {
        if(! __instance)
            __instance = new AuthManager();
        return __instance;
    }

    static register(observer) {
        AuthManager.instance().observers.push(observer);
    }

    static unregister(observer) {
        AuthManager.instance().observers.pop(observer);
    }

    static setUser(user) {
        AuthManager.user = user;
        AuthManager.instance().observers.forEach( (o) => o.setUser(user) );
    }

    static user() {
        return AuthManager.instance().user;
    }
}

export default AuthManager;
