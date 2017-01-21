let instance = null;

class LoginDispatcher {
    constructor() {
        if(! instance) {
            instance = this;
            this.observers = [];
        }
        return instance;
    }

    register(observer) {
        this.observers.push(observer);
    }

    unregister(observer) {
        this.observers.pop(observer);
    }

    setUser(user) {
        this.observers.forEach( (o) => o.setUser(user) );
    }
}

export default LoginDispatcher;
