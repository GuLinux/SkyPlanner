import React from 'react';
import AuthManager from './auth-manager';

class SkyPlannerHomePage extends React.Component {
    constructor(props) {
        super(props);
    }

    componentDidMount() {
        AuthManager.register(this);
    }

    componentWillUnmount() {
        AuthManager.unregister(this);
    }

    render() {
        return !!AuthManager.user() ? this.renderLoggedIn() : this.renderAnonymous(); 
    }

    renderAnonymous() {
        return <div>Hello, this is the Homepage</div>; 
    }

    renderLoggedIn() {
        return <div>Hello {AuthManager.user().username}, this is the Homepage</div>; 
    }

    loginChanged(user) {
        this.forceUpdate();
    }

}
export default SkyPlannerHomePage;
