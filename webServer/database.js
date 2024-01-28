const { default: mongoose, model } = require('mongoose')

//const mongoose=require('mongoose')
require('dotenv').config()

const database_url = process.env.DATABASE_URL

mongoose.connect(database_url,).then(_=>
    {
        console.log("database connected")
    }
).catch(e=>console.error(e))

const schema=new mongoose.Schema({
    "date": String,
    "duration": Number,
    "distance": Number,
    "avg_speed": Number,
    "altitude_diff": Number,
    "positions":[
        {
            "lat":String,
            "lng":String,
            "alt":String
        }
    ]
    
})
const positionModule=mongoose.model("positions",schema)

const insertTrk = async (obj)=>{
    const trk=new positionModule(obj)

    const e =await trk.save()
    console.log(e)

}

const findByDate= async (dateTemp)=>{
    const trk=positionModule.findOne({date: dateTemp})
    return trk
}
const findAll= async ()=>{
    const trk=positionModule.find({})
    return trk
}

module.exports={insertTrk,findByDate,findAll}