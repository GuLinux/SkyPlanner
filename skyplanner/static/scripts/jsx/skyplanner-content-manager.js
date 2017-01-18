import React from 'react';
class SkyPlannerContentManager extends React.Component {
    constructor(props, pages, current) {
        super(props);
        this.state['pages'] = pages;
        this.state['current'] = current;
    }

    render() {
        return <div>{this.currentPage()}</div>; 
    }

    currentPage() {
        return this.state['pages'][this.state['current']]['page'];
    }
}
export default SkyPlannerContentManager;
