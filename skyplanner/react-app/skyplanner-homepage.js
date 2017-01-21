import React from 'react';
import AuthManager from './auth-manager';

class SkyPlannerHomePage extends React.Component {
    constructor(props) {
        super(props);
        this.state = { user: null };
    }

    componentDidMount() {
        AuthManager.register(this);
    }

    componentWillUnmount() {
        AuthManager.unregister(this);
    }

    render() {
        return this.state.user ? this.renderLoggedIn() : this.renderAnonymous(); 
    }

    renderAnonymous() {
        return <div>Hello, this is the Homepage</div>; 
    }

    renderLoggedIn() {
        return <div>Hello {this.state.user.username}, this is the Homepage</div>; 
    }

    setUser(user) {
        console.log(user);
        this.setState({user: user});
    }

}
export default SkyPlannerHomePage;
