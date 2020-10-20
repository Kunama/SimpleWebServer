import axios from 'axios'
const base_url = "http://localhost:8888"

axios.get(`${base_url}/`).then(response => {
    console.log("GET",response.data.slice(0,100))
})
axios.post(`${base_url}/`, "This is the data being posted").then(response => {
    console.log("POST",response.data.slice(0,100))
})
