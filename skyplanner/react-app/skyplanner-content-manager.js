import React from 'react';
class SkyPlannerContentManager extends React.Component {
    constructor(props) {
        super(props);
        this.state = {current: 'home'};
    }

    render() {
        return <div className='container-fluid'>{this.current()}</div>; 
    }

    setCurrent(page) {
        setState({current: page});
    }

    current() {
        return this.props.children.find( (x) => x.key == this.state.current);
    }
}
export default SkyPlannerContentManager;
